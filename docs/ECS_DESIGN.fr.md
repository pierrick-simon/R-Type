# Notes de conception de l'ECS

> 🌐 **Langue :** Français · [English](ECS_DESIGN.en.md)

Ce document présente l'architecture de l'ECS du moteur (`Engine/System`) et, surtout, **les raisons** des choix retenus.

- **Public visé :** les personnes qui maintiennent ou font évoluer le moteur.
- **Pour utiliser l'ECS au quotidien**, consultez plutôt [le guide d'utilisation de l'ECS](ECS_GUIDE.fr.md).

Seules les signatures sont présentées, jamais le corps des fonctions : ce document décrit une architecture, pas une implémentation. Les signatures sont simplifiées (commentaires Doxygen, `[[nodiscard]]` et constructeurs de copie supprimés omis) ; la référence reste le code dans `Engine/System/include/`.

Les évolutions envisagées mais non implémentées sont listées à la fin, dans [Reste à faire](#reste-à-faire).

---

## Sommaire

- [Principes directeurs](#principes-directeurs)
- [Vue d'ensemble](#vue-densemble)
- [Architecture actuelle](#architecture-actuelle)
  - [Entity et EntityManager](#entity-et-entitymanager)
  - [Composants](#composants)
  - [IComponentArray et ComponentArray](#icomponentarray-et-componentarray)
  - [ComponentManager](#componentmanager)
  - [View](#view)
  - [ISystem, System et SystemManager](#isystem-system-et-systemmanager)
  - [GameManager (façade)](#gamemanager-façade)
  - [La file de commandes](#la-file-de-commandes)
- [Reste à faire](#reste-à-faire)

---

## Principes directeurs

1. **Les composants sont des données pures, sans classe de base commune.** Il n'existe pas de classe `Component`. L'héritage et le polymorphisme détruiraient la localité mémoire recherchée (stockage contigu par type) et imposeraient un stockage par pointeur (`unique_ptr<Component>`) au lieu de conteneurs denses.

2. **Un `ComponentArray<Component>` par type de composant**, organisé en *sparse set* : `dense` contient les données réelles, regroupées et rapides à parcourir ; `sparse` donne un accès direct en O(1) à partir d'une `Entity`. Les deux rôles sont volontairement séparés : `dense` ne contient que les composants qui existent, tandis que `sparse` peut contenir des trous (il est dimensionné sur la plage d'index d'entités).

3. **Une fonction virtuelle ne peut jamais dépendre du type `Component`.** C'est la règle qui guide tout le découpage entre interfaces (`IComponentArray`, `ISystem`) et classes templates (`ComponentArray<Component>`, `System<Components...>`) : une fonction virtuelle doit avoir une signature fixe, connue à la compilation pour construire la vtable, et ne peut donc pas être un template. Tout ce qui manipule le type concret vit uniquement sur la classe template, jamais sur l'interface.

4. **Les entités sont des handles générationnels, pas des objets.** Une `Entity` est un couple *(index, génération)*. L'index est recyclé ; la génération garantit qu'un ancien handle ne désignera jamais la nouvelle entité.

5. **Toute référence durable passe par un conteneur stable.** Les composants sont stockés dans un `plf::colony` et les systèmes dans une `std::list` : un élément ne change jamais d'adresse tant qu'il n'est pas lui-même supprimé.

6. **Aucune modification structurelle pendant un parcours.** Pendant `update()`, les opérations qui modifient la structure (destruction, ajout, retrait) sont différées dans une file de commandes et appliquées en fin de frame.

7. **`GameManager` est la seule façade publique.** `EntityManager`, `ComponentManager` et `SystemManager` sont des détails internes ; le code de jeu passe exclusivement par `GameManager`.

---

## Vue d'ensemble

```text
GameManager                      façade publique + file de commandes
├── ComponentManager             type de composant -> ComponentArray
│   └── ComponentArray<T>        sparse set (dense : plf::colony<ComponentPack>)
├── View<Ts...>                  parcourt les entités qui ont tous les Ts (jointure)
├── EntityManager                index + génération, recyclage des index
└── SystemManager                std::list de systèmes, ordre d'exécution
    └── System<Ts...>            boucle sur une View<Ts...> ; sans type : une fois par frame
```

Toutes les classes sont dans l'espace de noms `RType`.

---

## Architecture actuelle

### Entity et EntityManager

```cpp
class Entity {
public:
    explicit Entity(std::size_t index, std::size_t generation = 0);

    bool operator==(const Entity &entity) const;
    std::size_t index() const;
    std::size_t generation() const;

private:
    std::size_t _index;
    std::size_t _generation;
};

struct EntityInfo {
    bool isActive = true;
    std::size_t generation = 0;
};

using EntityList = std::vector<EntityInfo>;
using RecycleQueue = std::queue<Entity>;

class EntityManager {
public:
    Entity createEntity();
    void destroyEntity(Entity entity);
    bool hasEntity(Entity entity) const;

private:
    EntityList _entities;      // un emplacement par index déjà attribué
    RecycleQueue _toRecycle;   // index libérés, en attente de réutilisation
};
```

**Pourquoi une génération en plus de l'index.** Sans génération, un handle conservé après la destruction de son entité désignerait silencieusement la prochaine entité qui récupère le même index : un missile poursuivrait par exemple une cible qui n'a rien à voir. Avec le couple *(index, génération)*, `hasEntity` compare la génération du handle à celle de l'emplacement : un handle périmé est détecté au lieu d'être réinterprété.

**Pourquoi une file (FIFO) pour le recyclage.** L'index libéré le plus anciennement est réutilisé en premier. Un même index est donc réutilisé le plus tard possible, ce qui limite la fréquence d'incrémentation de sa génération.

**Pourquoi `Entity` n'a ni constructeur par défaut, ni `operator<`, ni `std::hash`.** L'absence de constructeur par défaut empêche de créer un handle « vide » qui désignerait implicitement l'index 0. Les opérations d'ordre et de hachage n'ont pas été nécessaires jusqu'ici ; elles pourront être ajoutées si un besoin apparaît.

> [!NOTE]
> Le constructeur `Entity(index, generation)` est public pour des raisons techniques (il est utilisé par `EntityManager`), mais le code de jeu ne doit jamais l'appeler : une `Entity` s'obtient uniquement via `createEntity()`.

### Composants

```cpp
struct Position {
    float x;
    float y;
};
```

Aucune classe de base, aucun constructeur ni destructeur personnalisé n'est nécessaire : une struct ordinaire suffit. La seule contrainte est d'être **déplaçable** ; les types non copiables sont acceptés.

### IComponentArray et ComponentArray

```cpp
class IComponentArray {
public:
    virtual ~IComponentArray() = default;
    virtual std::size_t nbComponent() const = 0;
    virtual void removeEntity(Entity entity) = 0;
    virtual bool hasComponent(Entity entity) const = 0;
};

template<typename Component>
class ComponentArray : public IComponentArray {
public:
    class ComponentPack {
    public:
        explicit ComponentPack(Entity entity, Component &&c);
        Entity entity() const;

        Component component;

    private:
        Entity _entity;
    };

    using Dense = plf::colony<ComponentPack>;

    template<typename... Args>
    Component &addEntity(Entity entity, Args &&...args);
    void removeEntity(Entity entity) override;
    Component &getComponent(Entity entity);
    bool hasComponent(Entity entity) const override;
    std::size_t nbComponent() const override;
    Dense &getDense();

private:
    Dense _dense;
    std::vector<typename Dense::iterator> _sparse;   // indexé par entity.index()
};
```

**Pourquoi `plf::colony` plutôt que `std::vector` pour `dense`.** Avec un `std::vector`, une suppression impose un *swap-and-pop* qui déplace le dernier élément : toute référence externe vers ce composant devient invalide, et `sparse` doit être resynchronisé. `plf::colony` (le conteneur dont est issu `std::hive`, standardisé en C++26) garantit qu'un élément non supprimé ne change jamais d'adresse, quelles que soient les insertions et suppressions ailleurs dans le conteneur. C'est ce qui permet à `sparse` de stocker directement des itérateurs, et au code de jeu de conserver une référence vers un composant (voir le guide, section 3).

**Comment `sparse` représente l'absence de composant.** Une case de `sparse` contient soit un itérateur valide vers `dense`, soit un itérateur construit par défaut, qui signifie « pas de composant ». `sparse` est agrandi à la demande, d'environ 1,5 fois l'index demandé, pour amortir les redimensionnements.

**Pourquoi `dense` stocke l'entité avec le composant (`ComponentPack`).** Un système sur plusieurs composants (`System<Position, Velocity>`) parcourt le `dense` d'un de ses types, puis doit retrouver les autres composants **de la même entité**. Or `sparse` ne fonctionne que dans le sens entité → composant : sans lien de l'élément vers son entité, le système ne peut pas savoir à qui appartient le composant qu'il parcourt. Un tableau d'entités séparé et parallèle à `dense` (le choix d'EnTT) n'est pas possible avec `plf::colony`, qui n'offre pas d'index stable partagé entre deux conteneurs ; l'entité est donc stockée dans l'élément lui-même. Le surcoût est d'au moins 16 octets par composant, acceptable pour le nombre d'entités d'un R-Type.

**Pourquoi l'`Entity` entière, et pas seulement son index.** L'index suffirait pour interroger les autres tableaux, mais un handle reconstruit à partir de l'index seul aurait une génération fausse : il serait considéré comme périmé dès qu'il sortirait du tableau (vers un système ou vers `GameManager`). Stocker l'`Entity` complète garantit que `pack.entity()` est toujours un vrai handle.

**Pourquoi l'entité est en lecture seule.** `getDense()` est public. Si l'entité d'un `ComponentPack` pouvait être modifiée, `dense` et `sparse` se désynchroniseraient. Elle est donc privée et exposée par un accesseur ; seul `component` est modifiable.

**Pourquoi `ComponentArray` ne vérifie pas la génération.** `sparse` est indexé par `entity.index()` uniquement. La vérification de la génération est faite en amont, une seule fois, par `GameManager` (`hasEntity`) ; et comme la destruction d'une entité retire tous ses composants, un emplacement recyclé ne contient jamais les composants de l'ancienne entité. **Invariant :** aucun code ne doit appeler `ComponentArray` avec un handle non vérifié.

**Pourquoi `hasComponent`, `removeEntity` et `nbComponent` sont virtuelles, mais pas `addEntity` ni `getComponent`.** Les trois premières ont une signature indépendante de `Component` : elles peuvent légitimement figurer sur l'interface. `addEntity` et `getComponent` manipulent un `Component &`, un type différent pour chaque `ComponentArray` : elles ne peuvent structurellement pas être virtuelles (principe n°3). `ComponentManager` les appelle directement sur le type concret.

### ComponentManager

```cpp
using ComponentList =
    std::unordered_map<std::type_index, std::unique_ptr<IComponentArray>>;

class ComponentManager {
public:
    template<typename Component, typename... Args>
    Component &addComponent(Entity entity, Args &&...args);

    template<typename Component>
    void removeComponent(Entity entity);

    void removeComponents(Entity entity);   // vide TOUS les ComponentArray pour cette entité

    template<typename Component>
    Component &getComponent(Entity entity);

    template<typename Component>
    ComponentArray<Component> &getComponentArray();

    template<typename Component>
    bool hasComponent(Entity entity) const;

    template<typename Component>
    bool isComponent() const;   // un ComponentArray<Component> existe-t-il ?

private:
    ComponentList _components;
};
```

**Pourquoi une `std::unordered_map<std::type_index, …>`.** `getComponentArray<Component>()` doit retrouver le tableau correspondant à un type donné. Avec un simple `std::vector`, il faudrait parcourir toute la liste en tentant un `dynamic_cast` sur chaque élément : lent et fragile. Avec une table indexée par `std::type_index(typeid(Component))`, la recherche est directe.

**Comment le type concret est retrouvé.** `getComponentArray<Component>()` cherche l'entrée sous la clé `type_index(typeid(Component))`, puis effectue un `static_cast<ComponentArray<Component> &>` sur l'`IComponentArray` stocké. Ce cast est sûr uniquement parce que `ComponentManager` garantit l'invariant suivant : une entrée sous cette clé n'est jamais créée autrement que comme un véritable `ComponentArray<Component>` (dans `addComponent<Component>`, qui crée le tableau s'il n'existe pas encore).

**Pourquoi un tableau n'est jamais supprimé.** Une fois créé, un `ComponentArray<Component>` reste en place même vide. Les références obtenues via `getArray<T>()` restent ainsi valides, et `isComponent<T>()` ne repasse jamais à `false`.

**Pourquoi `removeComponents(Entity)` existe en plus de `removeComponent<Component>`.** Lorsqu'une entité est détruite, ses composants doivent être retirés de *tous* les tableaux, sans connaître leurs types à cet endroit du code. C'est le cas d'usage principal du polymorphisme : on parcourt `_components` et on appelle `hasComponent` puis `removeEntity` via l'interface.

### View

```cpp
template<typename Component, typename... Components>
constexpr std::size_t duplicateCheck = (std::is_same_v<Component, Components> + ...);

template<typename... Components>
class View {
public:
    static_assert(((duplicateCheck<Components, Components...> == 1) && ...));

    explicit View(ComponentManager &cm);
    void each(std::function<void(Entity, Components &...)> f);
    std::vector<std::tuple<Entity, Components &...>> getVector();

private:
    std::tuple<ComponentArray<Components> *...> _view;   // nullptr si le type n'a jamais existé

    void iterateSmallest(std::function<void(Entity, Components &...)> &f,
        ComponentArray<Components> &...arrays);
    template<typename Array>
    void iterate(std::function<void(Entity, Components &...)> &f, Array &smallest,
        ComponentArray<Components> &...arrays);
};
```

**Pourquoi une vue.** Parcourir les entités qui possèdent *plusieurs* types de composants (la *jointure*) est utile partout : dans un système, dans un système par frame (collisions), ou hors de toute frame (réseau, rendu). La vue rend cette jointure accessible n'importe où, et donne l'entité en plus des composants. `System<Components...>` n'est plus qu'une boucle sur une vue.

**Comment la vue trouve les entités (la jointure).**

1. Le constructeur récupère, une seule fois, un pointeur vers le `ComponentArray` de chaque type.
2. `each` ouvre le tuple avec `std::apply` pour obtenir les tableaux sous forme de pack, puis appelle `iterateSmallest`.
3. `iterateSmallest` calcule la plus petite taille (`std::min` sur les `nbComponent()`), puis appelle `iterate` sur le **premier tableau de cette taille**, et sur lui seul (fold sur `||`, qui s'arrête au premier `true`).
4. `iterate` parcourt le `dense` de ce tableau ; chaque `ComponentPack` donne l'entité de l'élément. Si l'entité possède tous les autres types, la fonction est appelée avec l'entité et un composant de chaque type, obtenus en O(1) directement sur les tableaux (sans repasser par `ComponentManager`). Sinon, l'entité est ignorée.

**Pourquoi parcourir le plus petit tableau.** Chaque recherche est en O(1), mais il y en a une par élément du tableau parcouru : le coût dépend donc de la taille de ce tableau, et non de celle des autres. Pour `View<Position, Player>` avec 1000 positions et un seul joueur, parcourir `Position` coûterait 1000 recherches, dont 999 inutiles ; parcourir `Player` n'en coûte qu'une. Le choix est refait à chaque `each`, car les tailles évoluent en cours de partie.

**Pourquoi `iterate` est un template.** Le tableau à parcourir n'est connu qu'à l'exécution, mais son type (`ComponentArray<Position>` ou `ComponentArray<Player>`) doit être connu à la compilation : aucune variable ne peut désigner « l'un ou l'autre ». La boucle est donc écrite une seule fois, sous forme de template sur le type du tableau, et le fold l'instancie pour chaque type possible ; seule l'instance correspondant au plus petit tableau est appelée.

**Pourquoi des pointeurs, rangés dans un `std::tuple`.**

- **Des pointeurs plutôt que des références :** si un type n'a jamais été ajouté, `getComponentArray<T>()` lancerait une exception. La vue stocke alors `nullptr`, et `each` ne fait rien : une vue sur un type absent est vide, sans exception. Une référence ne peut pas valoir « rien ».
- **Un tuple :** un pack ne peut pas être un membre de classe (`ComponentArray<Components> *...arrays;` n'est pas du C++ valide). Le tuple range un pointeur par type, et `std::apply` le retransforme en pack au moment du parcours.

**Pourquoi interdire un type en double, et à la compilation.** Avec `View<Position, Position>`, la fonction recevrait deux références vers **le même** composant : une source de bogues silencieux (deux écritures qui s'écrasent), sans aucun cas d'usage légitime. Un `static_assert` transforme ce bogue en erreur de compilation, avec un message lisible. `duplicateCheck<T, Components...>` compte les occurrences de `T` dans le pack (un fold sur `+` de `std::is_same_v`) ; le fold externe sur `&&` exige que chaque type apparaisse exactement une fois. Comparer chaque type au seul premier ne suffirait pas : `<Position, Velocity, Velocity>` passerait. `System` porte le même `static_assert`, pour que l'erreur désigne `createSystem` et non une vue interne.

**Pourquoi une vue n'est pas un cache.** Elle ne stocke que des pointeurs vers les tableaux, jamais de liste d'entités : elle voit donc toujours l'état actuel, et la créer ne coûte que quelques recherches dans `ComponentManager`. Elle n'a pas à être invalidée ou mise à jour, mais elle est faite pour être utilisée immédiatement, puis jetée.

**Pourquoi `getVector` existe, et pourquoi il réutilise `each`.** `each` ne permet ni `break`, ni `return` vers l'appelant, ni de parcourir deux fois la même liste (boucles imbriquées des collisions). Un range-for le permet, mais il exige un `begin()`/`end()` : écrire un vrai itérateur est difficile ici, car le tableau parcouru est choisi à l'exécution parmi des `Dense` de types différents (voir [Reste à faire](#reste-à-faire)). `getVector` contourne le problème : il appelle `each` avec une lambda qui ajoute chaque entité et ses composants au vector, et la jointure n'est donc écrite qu'une fois. Le range-for parcourt ensuite un simple `std::vector`.

**Pourquoi des références dans le tuple.** `std::tuple<Entity, Components...>` copierait chaque composant : une écriture dans la boucle modifierait la copie, pas le composant, et un composant non copiable ne compilerait pas. Un tuple de références est légal dans un `std::vector` (il se construit et se déplace, ce qui suffit à `emplace_back`). Les éléments du vector étant des lvalues, `for (auto &[e, a, b] : ...)` compile, contrairement à un itérateur qui renverrait le tuple par valeur.

**Ce que coûte `getVector`, et pourquoi c'est sûr.** Une allocation par appel, et une photo de l'état au moment de l'appel. Les références restent valides tant que leur composant n'est pas retiré, puisque `plf::colony` ne déplace jamais un élément : détruire une entité pendant le parcours (même hors `update()`, où c'est immédiat) n'invalide que les références de cette entité, jamais le vector lui-même. C'est pourquoi le guide le recommande pour détruire des entités hors `update()`, à la place de « collecter puis détruire ».

**Pourquoi `each` prend un `std::function`.** C'est la forme la plus simple, et elle accepte les paramètres `const`. Le coût est un appel indirect par entité ; un paramètre template (`template<typename F> void each(F &&f)`) l'éviterait (voir [Reste à faire](#reste-à-faire)).

### ISystem, System et SystemManager

```cpp
class ISystem {
public:
    virtual ~ISystem() = default;
    virtual void update(ComponentManager &cm) = 0;
};

template<typename... Components>
class System : public ISystem {
public:
    static_assert(((duplicateCheck<Components, Components...> == 1) && ...));

    explicit System(std::function<void(Components &...)> &func);
    void update(ComponentManager &cm) override;   // une View<Components...>, ou _func() une fois si aucun type

private:
    std::function<void(Components &...)> _func;
};

using SystemId = std::size_t;

struct SystemEntry {
    SystemId id;
    std::unique_ptr<ISystem> system;
};

using SystemList = std::list<SystemEntry>;

class SystemManager {
public:
    void process(ComponentManager &cm);

    template<typename... Components>
    SystemId createSystem(std::function<void(Components &...)> &func);

    void destroySystem(SystemId id);
    bool hasSystem(SystemId id) const;

private:
    SystemList _systems;
    SystemId _nextId = 0;
};
```

**Pourquoi `update` reçoit un `ComponentManager &`.** Un système a besoin d'accéder aux données typées (le `ComponentArray` de chacun de ses types), pas à une liste brute d'identifiants. `ComponentManager` est le seul point d'accès pertinent. Si l'un des types n'a jamais existé (`isComponent<T>()` est faux pour l'un d'eux), le système ne fait rien.

**Comment `System<Components...>` trouve les entités.** `update` crée une `View<Components...>` et appelle `each` avec une lambda qui ignore l'entité et appelle `_func` avec les composants. Toute la jointure (plus petit tableau, types absents, doublons) est donc celle de la vue : voir [View](#view).

**Pourquoi un système sans type est un système par frame.** Certains traitements ne portent pas sur *une* entité : les collisions entre deux groupes, l'apparition des vagues, le score. Plutôt qu'une deuxième API, un pack vide prend ce sens : avec `Components` vide, `std::function<void(Components &...)>` devient naturellement `std::function<void()>`, et `update` appelle `_func()` une seule fois. Le système garde tout le reste : son `SystemId`, son rang dans l'ordre d'exécution, et la protection de la file de commandes. Grâce à `std::type_identity_t`, le pack est déduit comme vide quand aucun type n'est donné : `createSystem(func)` et `createSystem<>(func)` sont équivalents. Un système par frame peut aussi recevoir le `GameManager` (voir [GameManager (façade)](#gamemanager-façade)) : c'est la forme recommandée pour les collisions, ou pour tout système qui a besoin de l'entité qu'il traite, en parcourant une vue.

**Pourquoi `if constexpr` dans `update`.** Avec un `if` classique, les deux branches seraient compilées pour **chaque** `System` : `_func()` ne compilerait pas pour un `System<Health>` (sa fonction attend un `Health &`), et `View<>` ne compilerait pas pour un `System<>` (`std::min` sur une liste vide). `if constexpr` écarte de la compilation la branche qui ne correspond pas au pack ; le `else` est indispensable pour que la seconde branche soit elle aussi écartée.

**Pourquoi `createSystem` renvoie un `SystemId` plutôt qu'un pointeur ou un itérateur.**

- Renvoyer un `std::unique_ptr<ISystem>` transférerait la propriété à l'appelant ; or `SystemManager` doit rester l'unique propriétaire pour pouvoir exécuter le système dans `process()`.
- Un pointeur brut ou un itérateur deviendrait invalide dès la destruction du système, sans aucun moyen de le détecter.
- Un identifiant numérique, lui, peut toujours être vérifié (`hasSystem`). Comme les identifiants sont attribués par un compteur croissant (`_nextId`) et **jamais réutilisés**, un ancien identifiant ne peut jamais désigner un autre système.

**Pourquoi une `std::list`.**

- **L'ordre d'exécution est l'ordre de création** : les systèmes sont ajoutés en fin de liste et parcourus dans l'ordre.
- **Les itérateurs restent valides lors d'un ajout** : un `createSystem` appelé pendant `process()` ne corrompt pas le parcours en cours (le nouveau système s'exécute simplement dès cette frame). Avec un `std::vector`, une réallocation invaliderait le parcours.
- La recherche par identifiant est linéaire, ce qui est acceptable : le nombre de systèmes est faible et `destroySystem`/`hasSystem` ne sont pas appelés à chaque frame.

**Pourquoi la destruction d'un système est différée pendant `update()`.** Retirer un nœud de la liste pendant son parcours invaliderait l'itérateur courant. `GameManager` place donc `destroySystem` dans la file de commandes (voir ci-dessous).

### GameManager (façade)

```cpp
using Command = std::function<void()>;
using CommandQueue = std::vector<Command>;

class GameManager {
public:
    void update();

    Entity createEntity();
    void destroyEntity(Entity entity);
    bool hasEntity(Entity entity) const;

    template<typename Component, typename... Args>
    void addComponent(Entity entity, Args &&...args);
    template<typename Component>
    void removeComponent(Entity entity);
    void removeComponents(Entity entity);
    template<typename Component>
    Component &getComponent(Entity entity);
    template<typename Component>
    bool hasComponent(Entity entity) const;
    template<typename Component>
    ComponentArray<Component> &getArray();

    template<typename... Components>
    View<Components...> view();

    template<typename... Components>
    SystemId createSystem(
        std::type_identity_t<std::function<void(Components &...)>> func);
    template<typename... Components>
    SystemId createSystem(const std::type_identity_t<
        std::function<void(GameManager &, Components &...)>> &func);
    void destroySystem(SystemId id);
    bool hasSystem(SystemId id) const;

private:
    ComponentManager _cm;
    EntityManager _em;
    SystemManager _sm;
    CommandQueue _cq;
    bool _updating = false;

    void flush();
    void tryFlush();
};
```

**Pourquoi une façade unique.** C'est la seule classe que le code de jeu doit connaître. Elle centralise aussi les vérifications transverses : chaque accès à un composant vérifie d'abord `hasEntity` (et donc la génération), ce qui permet aux classes internes de s'en dispenser.

**Pourquoi la copie est interdite.** Les commandes en attente et les adaptateurs des systèmes qui reçoivent le `GameManager` capturent `this`. Une copie partagerait ces captures avec l'original, ce qui mènerait à des accès vers le mauvais objet. Comme aucun constructeur de déplacement n'est déclaré, un `GameManager` ne peut pas non plus être déplacé : son adresse ne change jamais, et le `this` capturé reste valide aussi longtemps que lui.

**Pourquoi un système peut recevoir le `GameManager`.** Un système qui agit sur le monde (créer ou détruire des entités, lire les composants d'une autre entité, parcourir une vue) a besoin du `GameManager`. Le capturer dans une lambda (`[&gm]`) fonctionne, mais empêche d'écrire le système comme une fonction ordinaire, dans un fichier à part. `createSystem` accepte donc aussi une fonction dont le premier paramètre est un `GameManager &`.

- **Deux surcharges, sans ambiguïté.** La seconde surcharge prend un `std::function<void(GameManager &, Components &...)>`. Le constructeur de `std::function` n'accepte que les fonctions appelables avec ses arguments : pour une lambda ou une fonction donnée, une seule des deux surcharges est donc viable, et le compilateur la choisit seul. Une lambda générique (`[](auto &...)`), qui accepterait les deux formes, rend l'appel ambigu : c'est une erreur de compilation, ce qui évite un choix arbitraire.
- **Un adaptateur, construit une seule fois.** Le `GameManager` est connu dès la création du système : c'est `this`. La seconde surcharge enveloppe donc la fonction dans une lambda de la forme habituelle `(Components &...)`, qui appelle `func(*this, c...)`. `System` et `SystemManager` ne voient que la forme habituelle et n'ont pas changé ; avec un pack vide, l'adaptateur devient une lambda sans paramètre, et le système par frame fonctionne sans cas particulier.
- **La fonction est capturée par valeur.** `func` est un paramètre de `createSystem`, détruit dès son retour ; l'adaptateur, lui, vit aussi longtemps que le système. Il doit donc garder sa propre copie de la fonction : une capture par référence laisserait une référence pendante, appelée à la première frame.

**Pourquoi `std::type_identity_t` dans `createSystem`.** Les types sont donnés explicitement (`createSystem<Position, Velocity>(lambda)`), mais pour un paquet de paramètres, le compilateur tente quand même d'en déduire d'autres à partir de l'argument. Une lambda n'étant pas un `std::function`, cette déduction échoue et l'appel ne compile pas. `std::type_identity_t` rend le paramètre non déduit, dans les deux surcharges : seuls les types explicites sont utilisés, et la lambda est convertie en `std::function`.

### La file de commandes

**Le problème.** Pendant `update()`, chaque système parcourt le `dense` d'un `ComponentArray`. Si un système détruisait une entité ou ajoutait un composant pendant ce parcours, il modifierait le conteneur en cours de lecture, ou la liste des systèmes elle-même.

**La solution.** Les opérations structurelles (`destroyEntity`, `addComponent`, `removeComponent`, `removeComponents`, `destroySystem`) ne s'exécutent pas directement : elles ajoutent une `Command` (une lambda) dans `_cq`, puis appellent `tryFlush()`.

- **Hors `update()`** (`_updating` est faux), `tryFlush()` exécute immédiatement la file : l'opération paraît synchrone.
- **Pendant `update()`**, `tryFlush()` ne fait rien ; la file est exécutée par `flush()` à la fin d'`update()`, dans l'ordre des demandes.

Les opérations qui ne modifient pas la structure (`createEntity`, les lectures) restent immédiates. `createEntity` peut l'être sans risque car il ne touche à aucun conteneur parcouru par les systèmes ; et comme les destructions sont différées, un index libéré pendant la frame n'est jamais réattribué dans cette même frame.

**Pourquoi les vérifications sont faites dans la commande, et non au moment de l'appel.** L'état peut changer entre la demande et l'exécution (par exemple, deux systèmes détruisent la même entité). Chaque commande revérifie donc `hasEntity`/`hasSystem` au moment où elle s'exécute : une destruction en double n'a aucun effet, et un ajout sur une entité entre-temps détruite lance `NoSuchEntity`.

**Pourquoi le composant est construit immédiatement par `addComponent`.** Le composant est construit à l'appel, puis déplacé dans son tableau lors de l'exécution de la commande. Une exception levée par le constructeur sort donc directement de l'appel à `addComponent`, là où elle est compréhensible. Le composant est conservé dans un `std::shared_ptr` capturé par la lambda, car `std::function` exige des objets copiables : capturer directement un composant non copiable (contenant un `std::unique_ptr`, par exemple) ne compilerait pas.

**Garanties en cas d'exception.**

- `flush()` exécute **toutes** les commandes, même si certaines échouent ; il conserve la première exception rencontrée et la relance après avoir vidé la file.
- Si un système lance une exception, `update()` exécute quand même la file (en ignorant les erreurs des commandes) puis relance l'exception du système. La frame est ainsi toujours appliquée, et l'ECS reste dans un état cohérent pour la frame suivante.
- `update()` n'est pas réentrant : un appel depuis un système (`_updating` vrai) n'a aucun effet.

> [!WARNING]
> Une commande ne doit jamais rappeler le `GameManager` pendant son exécution, ce qui inclut les constructeurs et destructeurs de composants : ajouter une commande à `_cq` pendant que `flush()` parcourt ce même vecteur provoquerait une réallocation en plein parcours, ou une récursion infinie de `flush()`. C'est la règle documentée dans le guide (section 7, piège n°1).

---

## Reste à faire

Ces évolutions ne sont **pas implémentées**. Elles ne sont décrites ici que dans leur principe.

### Un pointeur au lieu d'un itérateur dans `sparse`

`sparse` stocke aujourd'hui un `plf::colony::iterator` par index d'entité, soit 24 octets par case, pour chaque type de composant, y compris pour les entités qui n'ont pas ce composant. Un simple `ComponentPack *` (8 octets, `nullptr` pour « pas de composant ») suffirait : `plf::colony` garantit qu'un élément ne change pas d'adresse tant qu'il n'est pas supprimé. Seul `removeEntity` a besoin d'un itérateur, pour `erase` : il peut le retrouver avec `_dense.get_iterator(pointeur)`, une opération plus lente qu'un accès direct, mais rare et toujours différée.

### Recevoir l'entité dans la fonction d'un système

Permettre `createSystem<Lifetime>([](Entity e, Lifetime &l) {...})`, en plus des formes actuelles. En attendant, un système par frame qui parcourt une vue fournit déjà l'entité : c'est la pratique recommandée par le guide, et elle évite de stocker l'entité dans ses propres composants. La difficulté est d'accepter **plusieurs** formes de fonction, et contrairement au `GameManager`, l'entité n'est connue qu'au moment du parcours : l'adaptateur doit donc vivre dans `System`, pas dans `GameManager`. `createSystem` devrait prendre un appelable quelconque (`template<typename... Components, typename F>`), détecter la forme avec `std::is_invocable_v`, et envelopper la forme sans entité dans une lambda qui l'ignore (en capturant la fonction **par valeur**). `System` stockerait alors toujours la forme avec entité, sauf pour un système par frame, qui n'a pas d'entité (`std::conditional_t`). Une lambda générique, qui accepte les deux formes, imposerait de choisir une priorité.

### Améliorer la vue

- **La syntaxe `for` sans allocation** : `getVector()` donne déjà la boucle `for`, au prix d'un vector alloué à chaque appel. Un vrai itérateur (`for (auto [e, pos, vel] : gm.view<Position, Velocity>())`) l'éviterait : il encapsulerait « parcourir le plus petit tableau et sauter les entités incomplètes ». La difficulté est que le tableau parcouru est choisi à l'exécution, parmi des `Dense` de types différents : l'itérateur devrait contenir un `std::variant` des itérateurs de colony, ou bien chaque tableau devrait exposer une liste d'entités de type commun (comme EnTT). `operator*` renverrait alors le tuple par valeur, ce qui impose `auto` ou `auto &&` dans la boucle, et non `auto &`.
- **`each` en template** : `template<typename F> void each(F &&f)` éviterait l'appel indirect du `std::function` pour chaque entité.
- **Les paires** : `eachPair`, qui donne chaque paire d'entités une seule fois (comme `iter_combinations` dans Bevy), pour les collisions entre entités d'un même groupe.

### Activer et désactiver une entité

Permettre de mettre en pause une entité (gel d'un ennemi, cinématique, animation de mort…), pour tous les systèmes ou pour un seul, sans retirer ses composants. Le prérequis est rempli, puisque chaque `ComponentPack` connaît son entité : un système peut donc vérifier, pour chaque entité parcourue, la présence d'un composant « tag » de désactivation. Deux points restent à concevoir :

- **comment désigner un système précis** : `System<Component, Tag>` n'est plus possible, car `System` est variadique et le lirait comme un système sur deux composants. Il faudra une classe distincte, ou désigner le système par son `SystemId` ;
- **filtrer sur l'absence d'un composant** : les vues et les systèmes filtrent déjà sur la *présence* d'un tag, mais pas encore sur son *absence*.

En attendant, ajouter et retirer un composant « tag » couvre la plupart des cas (voir la recette « Mettre une entité en pause » dans le guide, section 8).
