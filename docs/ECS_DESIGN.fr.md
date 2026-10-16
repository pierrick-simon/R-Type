# Notes de conception de l'ECS

> 🌐 **Langue :** Français · [English](ECS_DESIGN.en.md)

Ce document présente l'architecture de l'ECS du moteur (`Engine/System`) et, surtout, **les raisons** des choix retenus.

- **Public visé :** les personnes qui maintiennent ou font évoluer le moteur.
- **Pour utiliser l'ECS au quotidien**, consultez plutôt [le guide d'utilisation de l'ECS](ECS_GUIDE.fr.md).

Seules les signatures sont présentées, jamais le corps des fonctions : ce document décrit une architecture, pas une implémentation. Les signatures sont simplifiées (commentaires Doxygen, `[[nodiscard]]` et constructeurs de copie supprimés omis) ; la référence reste le code dans `Engine/System/include/`.

Le document est organisé en deux parties :

- **[Partie 1](#partie-1--architecture-actuelle)** : l'architecture actuellement implémentée.
- **[Partie 2](#partie-2--extension-envisagée--activation-et-désactivation-par-entité)** : une extension envisagée (activation/désactivation par entité), **non implémentée**, à ne construire que si un besoin concret apparaît.

---

## Sommaire

- [Principes directeurs](#principes-directeurs)
- [Vue d'ensemble](#vue-densemble)
- [Partie 1 — Architecture actuelle](#partie-1--architecture-actuelle)
  - [Entity et EntityManager](#entity-et-entitymanager)
  - [Composants](#composants)
  - [IComponentArray et ComponentArray](#icomponentarray-et-componentarray)
  - [ComponentManager](#componentmanager)
  - [ISystem, System et SystemManager](#isystem-system-et-systemmanager)
  - [GameManager (façade)](#gamemanager-façade)
  - [La file de commandes](#la-file-de-commandes)
- [Partie 2 — Extension envisagée : activation et désactivation par entité](#partie-2--extension-envisagée--activation-et-désactivation-par-entité)

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
├── EntityManager                index + génération, recyclage des index
└── SystemManager                std::list de systèmes, ordre d'exécution
    └── System<Ts...>            appelle une fonction sur chaque entité qui a tous les Ts
```

Toutes les classes sont dans l'espace de noms `RType`.

---

## Partie 1 — Architecture actuelle

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

**Pourquoi `plf::colony` plutôt que `std::vector` pour `dense`.** Avec un `std::vector`, une suppression impose un *swap-and-pop* qui déplace le dernier élément : toute référence externe vers ce composant devient invalide, et `sparse` doit être resynchronisé. `plf::colony` (l'implémentation de référence de `std::hive`, standardisé en C++26) garantit qu'un élément non supprimé ne change jamais d'adresse, quelles que soient les insertions et suppressions ailleurs dans le conteneur. C'est ce qui permet à `sparse` de stocker directement des itérateurs, et au code de jeu de conserver une référence vers un composant (voir le guide, section 3).

**Comment `sparse` représente l'absence de composant.** Une case de `sparse` contient soit un itérateur valide vers `dense`, soit un itérateur construit par défaut, qui signifie « pas de composant ». `sparse` est agrandi à la demande, d'environ 1,5 fois l'index demandé, pour amortir les redimensionnements.

**Pourquoi `dense` stocke l'entité avec le composant (`ComponentPack`).** Un système sur plusieurs composants (`System<Position, Velocity>`) parcourt le `dense` du premier type, puis doit retrouver les autres composants **de la même entité**. Or `sparse` ne fonctionne que dans le sens entité → composant : sans lien de l'élément vers son entité, le système ne peut pas savoir à qui appartient le composant qu'il parcourt. Un tableau d'entités séparé et parallèle à `dense` (le choix d'EnTT) n'est pas possible avec `plf::colony`, qui n'offre pas d'index stable partagé entre deux conteneurs ; l'entité est donc stockée dans l'élément lui-même. Le surcoût est d'au moins 16 octets par composant, acceptable pour le nombre d'entités d'un R-Type.

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
    explicit System(std::function<void(Components &...)> &func);
    void update(ComponentManager &cm) override;   // appelle _func sur chaque entité qui a tous les Components

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

**Comment `System<Components...>` trouve les entités (la jointure).**

1. Les `ComponentArray` de tous les types sont récupérés une seule fois, avant la boucle (`std::tie`).
2. Le `dense` du **premier** type est parcouru ; chaque `ComponentPack` donne l'entité de l'élément.
3. Si l'entité possède tous les autres types, la fonction est appelée avec un composant de chaque type, obtenus en O(1) via leurs `sparse`. Sinon, l'entité est ignorée.

Chaque recherche est en O(1), mais il y en a une par élément du **premier** tableau : le coût d'une frame dépend donc de la taille de ce tableau, et non de celle des autres. C'est pourquoi le guide recommande de placer le type le plus rare en premier. Choisir automatiquement le plus petit tableau à chaque `update()` est une amélioration possible.

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
    SystemId createSystem(
        std::type_identity_t<std::function<void(Components &...)>> func);
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

**Pourquoi la copie est interdite.** Les commandes en attente et les fonctions des systèmes capturent souvent `this` ou une référence vers le `GameManager`. Une copie partagerait ces captures avec l'original, ce qui mènerait à des accès vers le mauvais objet.

**Pourquoi `std::type_identity_t` dans `createSystem`.** Les types sont donnés explicitement (`createSystem<Position, Velocity>(lambda)`), mais pour un paquet de paramètres, le compilateur tente quand même d'en déduire d'autres à partir de l'argument. Une lambda n'étant pas un `std::function`, cette déduction échoue et l'appel ne compile pas. `std::type_identity_t` rend le paramètre non déduit : seuls les types explicites sont utilisés, et la lambda est convertie en `std::function`.

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

## Partie 2 — Extension envisagée : activation et désactivation par entité

> [!IMPORTANT]
> **Non implémentée.** Cette extension ne doit pas être construite par anticipation : elle se conçoit mieux une fois le besoin précisément connu (gel d'un ennemi, cinématique, animation de mort…). Une alternative plus simple couvre déjà la plupart des cas : ajouter et retirer un composant « tag » (voir la recette « Mettre une entité en pause » dans le guide, section 8).

Si le besoin se confirme, voici le design envisagé.

### Deux niveaux de désactivation

```cpp
struct Disabled {};   // désactive TOUS les systèmes pour cette entité

template<typename Component>
struct DisabledSystem;   // déclarée mais jamais définie par défaut : type incomplet

// Spécialisation explicite, uniquement pour les composants dont le système est désactivable
template<>
struct DisabledSystem<Gravity> {};
```

**Pourquoi laisser `DisabledSystem<Component>` non définie par défaut.** Cela transforme « ce composant n'a pas de système désactivable » en erreur de compilation plutôt qu'en bogue silencieux à l'exécution : `addComponent<DisabledSystem<Velocity>>(entity)` ne compile pas tant que `DisabledSystem<Velocity>` n'a pas été explicitement spécialisée, car un type incomplet ne peut pas être stocké dans un `ComponentArray`.

### Prérequis : connaître l'entité pendant le parcours

Pour vérifier si une entité est désactivée, un système doit connaître l'entité du composant qu'il traite. **Ce prérequis est rempli** : chaque élément de `dense` est un `ComponentPack` qui contient son entité (voir [IComponentArray et ComponentArray](#icomponentarray-et-componentarray)).

### System\<Component, D\>

> [!WARNING]
> **Conflit avec le `System` actuel.** Depuis que `System` est variadique (`System<Components...>`), `System<Component, D>` désignerait un système sur **deux composants**, et non un système accompagné d'un tag. Il en va de même pour `createSystem<Component, D>`. Ce design devra être adapté avant toute implémentation, par exemple avec une classe et une fonction au nom distinct. À noter : un système sur plusieurs composants permet déjà de filtrer sur la **présence** d'un tag, mais pas sur son **absence**.

```cpp
template<typename Component, typename D>
class System : public ISystem {
public:
    explicit System(std::function<void(Component &)> &func);
    void update(ComponentManager &cm) override;

private:
    std::function<void(Component &)> _func;
};
```

`D` est le tag propre à ce système (typiquement `DisabledSystem<Component>`). Pour chaque entité parcourue, `update` vérifie :

1. `hasComponent<Disabled>(entity)` : désactivation globale ;
2. `hasComponent<D>(entity)` : désactivation de ce système précis.

**Pourquoi centraliser la vérification de `Disabled` dans `ISystem`.** `System<Components...>` (sans `D`) doit *aussi* respecter `Disabled` ; sinon, la désactivation globale n'aurait aucun effet sur les systèmes créés sans tag spécifique. Pour ne pas dupliquer (ou oublier) cette vérification dans chaque variante :

```cpp
class ISystem {
public:
    virtual ~ISystem() = default;
    virtual void update(ComponentManager &cm) = 0;

protected:
    bool isEntityActive(ComponentManager &cm, Entity entity) const;
    // implémentation : !cm.hasComponent<Disabled>(entity)
};
```

### API envisagée sur GameManager

```cpp
template<typename Component, typename D>
SystemId createSystem(std::function<void(Component &)> func);

template<typename Component>
void enable(Entity entity);    // removeComponent<DisabledSystem<Component>>(entity)
template<typename Component>
void disable(Entity entity);   // addComponent<DisabledSystem<Component>>(entity)
template<typename Component>
bool isEnabled(Entity entity) const;

void enable(Entity entity);    // removeComponent<Disabled>(entity)
void disable(Entity entity);   // addComponent<Disabled>(entity)
bool isEnabled(Entity entity) const;
```

### Limite connue

`enable<Component>` et `disable<Component>` distinguent les systèmes uniquement **par type de composant**, pas par instance. Si deux systèmes différents traitent un jour le même composant `Gravity` (par exemple un pour la physique et un pour un affichage de débogage), il sera **impossible** de les désactiver indépendamment avec cette API : `disable<Gravity>` les couperait tous les deux. Tant qu'il n'existe qu'un seul système par type de composant, ce n'est pas un problème. Dans le cas contraire, il faudra indexer la désactivation par `SystemId` plutôt que par type.

### Note de performance

Dans `System<Component, D>::update`, il faut récupérer `getComponentArray<Disabled>()` et `getComponentArray<DisabledSystem<Component>>()` **une seule fois avant la boucle**, et non à chaque entité. Sinon, chaque `hasComponent<T>` refait une recherche dans la table `type_index` de `ComponentManager` à chaque itération, pour chaque système, à chaque frame.
