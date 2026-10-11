# ECS – Notes de conception

Ce document résume les choix d'architecture retenus pour l'ECS (`Engine/System`), et **pourquoi** ils ont été faits. Il complète (et à certains endroits corrige) le squelette actuel dans `include/` — les écarts avec le code existant sont signalés explicitement. Seules les signatures sont données, jamais le corps des fonctions : ce fichier décrit une architecture, pas une implémentation.

Il est volontairement coupé en deux parties :
- **Partie 1** : la base de l'ECS, sans enable/disable. C'est ce qu'il faut implémenter en premier.
- **Partie 2** (fin du fichier) : l'extension enable/disable par entité, laissée en option — à ne construire que si un besoin concret se présente (voir la section correspondante en fin de document).

---

## Principes directeurs

1. **Les composants sont des données pures (POD), sans héritage commun.** Pas de classe de base `Component`. L'héritage + polymorphisme casserait la localité mémoire qu'on cherche justement à obtenir (stockage contigu par type), et forcerait du stockage par pointeur (`unique_ptr<Component>`) au lieu de tableaux denses.

2. **Un `ComponentArray<Component>` par type de composant** (sparse set). `dense` contient les données réelles, contiguës et itérables rapidement ; `sparse` permet un accès direct O(1) par `Entity`. Les deux rôles sont séparés exprès : `dense` doit rester strictement compact (seulement les composants qui existent), `sparse` peut avoir des trous (dimensionné sur la plage d'IDs possibles).

3. **Une seule fonction virtuelle ne peut jamais dépendre de `Component`.** C'est la règle qui a guidé tout le découpage `IComponentArray` / `ComponentArray<Component>` : une méthode virtuelle doit avoir une signature fixe (connue à la compilation pour construire la vtable), donc elle ne peut pas être un template. Tout ce qui a besoin de connaître le type concret (`addComponent`, `getComponent`, `getDense`) vit uniquement sur `ComponentArray<Component>`, jamais sur l'interface.

4. **Les entités sont de simples IDs recyclés**, pas des objets. Pas de liste exhaustive des entités vivantes à maintenir : un compteur + une pile d'IDs libérés suffisent (déjà le cas dans `EntityManager` actuel — bon choix, à garder).

5. **Les systèmes vivent dans un conteneur à références stables** (`plf::hive`, pas `std::vector`), parce qu'on a besoin de rendre un identifiant (`createSystem`) utilisable plus tard (`destroySystem`) sans qu'il soit invalidé par d'autres insertions/suppressions entre-temps.

6. **`Registry` est la seule façade publique.** `EntityManager`, `ComponentManager`, `SystemManager` restent des détails internes ; tout passe par `Registry`.

---

## Partie 1 — Base de l'ECS (sans enable/disable)

### Entity / EntityManager

```cpp
namespace ECS {

using Entity = std::uint32_t;

class EntityManager {
public:
    Entity create();
    void release(Entity entity);

private:
    std::vector<Entity> mFree;   // IDs libérés, recyclables
    Entity mNextEntity;          // prochain ID jamais utilisé
};

}
```

**Pourquoi pas une liste de toutes les entités vivantes ?** Parce qu'une `Entity` n'est qu'un entier — rien à itérer dessus directement d'utile en pratique (ce sont les `ComponentArray<Component>` qui savent quelles entités existent, via leur `sparse`/`dense`). Le couple compteur + pile de recyclage suffit à garantir des IDs uniques et réutilisables, sans structure supplémentaire à synchroniser. C'est déjà l'approche du code actuel — à conserver telle quelle.

### Component — exemple

```cpp
struct Position2d {
    int x;
    int y;
};
```

Aucune classe de base, aucun constructeur/destructeur custom nécessaire : une struct POD ordinaire.

### IComponentArray / ComponentArray\<Component\>

```cpp
class IComponentArray {
public:
    virtual ~IComponentArray() = default;
    virtual void destroyEntity(Entity entity) = 0;  // utilisé par ComponentManager::destroyEntity
    virtual bool hasComponent(Entity entity) const = 0;
};

template<typename Component>
class ComponentArray : public IComponentArray {
public:
    Component &addComponent(Entity entity, Component component);
    void removeComponent(Entity entity);
    Component &getComponent(Entity entity);
    bool hasComponent(Entity entity) const override;
    void destroyEntity(Entity entity) override;

    plf::hive<std::pair<Entity, Component>> &getDense();
    std::size_t size() const;

private:
    plf::hive<std::pair<Entity, Component>> mDense;
    std::vector<plf::hive<std::pair<Entity, Component>>::iterator> mSparse;
};
```

**Pourquoi `dense` stocke `std::pair<Entity, Component>` et pas juste `Component`** : un système qui itère `getDense()` a besoin de savoir à quelle `Entity` appartient chaque élément (par exemple pour vérifier un composant associé à la même entité, ou — voir Partie 2 — pour vérifier si cette entité est désactivée). Faire porter l'`Entity` directement dans `dense` évite d'avoir un second conteneur parallèle (`mDenseToEntity`) à garder synchronisé avec `dense`, ce qui serait plus fragile.

**Pourquoi `plf::hive` plutôt que `std::vector` pour `dense`** *(écart avec le code actuel, qui utilise `std::vector`)* : avec un `vector`, une suppression implique un swap-and-pop qui déplace le dernier élément — ce qui invalide toute référence/pointeur externe vers les composants en cours de traitement, et complique la synchronisation avec `mSparse`. `plf::hive` garantit qu'un élément non supprimé ne bouge jamais, même quand d'autres sont insérés/supprimés ailleurs dans la structure — exactement ce qu'il faut pour des références stables (et, par la même occasion, les handles de `SystemManager` plus bas réutilisent ce même principe).

**Pourquoi `hasComponent` est virtuelle sur `IComponentArray` mais pas `addComponent`/`getComponent`** : `hasComponent(Entity) -> bool` a une signature fixe, indépendante de `Component` — légitimement virtuelle. `addComponent`/`getComponent` ont besoin de manipuler `Component&` — un type qui change selon le `ComponentArray`, donc ils ne peuvent structurellement pas être virtuels (règle n°3 plus haut). `ComponentManager` les appelle directement sur le type concret, jamais via l'interface.

### ComponentManager

```cpp
using ComponentList = std::unordered_map<std::type_index, std::unique_ptr<IComponentArray>>;

class ComponentManager {
public:
    template<typename Component>
    Component &addComponent(Entity entity, Component component);

    template<typename Component>
    void removeComponent(Entity entity);

    template<typename Component>
    Component &getComponent(Entity entity);

    template<typename Component>
    bool hasComponent(Entity entity);

    template<typename Component>
    ComponentArray<Component> &getArray();

    void destroyEntity(Entity entity);  // nettoie TOUS les ComponentArray pour cette entité

private:
    ComponentList mComponentArrays;
};
```

**Pourquoi `std::unordered_map<std::type_index, ...>` plutôt qu'un `std::vector`** *(écart avec le code actuel, qui utilise `std::vector<std::unique_ptr<IComponentArray>>`)* : `getArray<Component>()` a besoin de retrouver le `ComponentArray<Component>` correspondant à un type donné. Avec un `vector`, il faudrait parcourir toute la liste et faire un `dynamic_cast` sur chaque élément pour deviner lequel correspond — lent et fragile. Avec `unordered_map<type_index, ...>`, le lookup par type est O(1) direct via `typeid(Component)`.

**Comment `getArray<Component>()` retrouve le type concret** : en interne, elle cherche l'entrée sous la clé `type_index(typeid(Component))`, puis fait un `static_cast<ComponentArray<Component>&>` sur le `IComponentArray*` stocké. Ce cast est sûr uniquement parce que `ComponentManager` garantit lui-même l'invariant : une entrée sous cette clé n'est jamais créée que comme un vrai `ComponentArray<Component>` (dans `addComponent<Component>`, qui crée l'entrée si elle n'existe pas encore).

**Pourquoi `destroyEntity(Entity)` existe en plus de `removeComponent<Component>`** : quand une entité est détruite, il faut purger ses composants dans *tous* les `ComponentArray`, sans connaître leurs types un par un à cet endroit du code. C'est le seul endroit où le polymorphisme (`IComponentArray::destroyEntity`) sert réellement — on boucle sur `mComponentArrays` et on appelle la version virtuelle sur chacun.

### ISystem / System\<T\> / SystemManager

```cpp
class ISystem {
public:
    virtual ~ISystem() = default;
    virtual void update(ComponentManager &cm) = 0;
};

template<typename Component>
class System : public ISystem {
public:
    explicit System(std::function<void(Component &)> func);
    void update(ComponentManager &cm) override;

private:
    std::function<void(Component &)> mFunc;
};

using SystemList = plf::hive<std::unique_ptr<ISystem>>;

class SystemManager {
public:
    template<typename Component>
    SystemList::iterator createSystem(std::function<void(Component &)> func);

    void destroySystem(SystemList::iterator system);

    void process(ComponentManager &cm);

private:
    SystemList mSystems;
};
```

**Pourquoi `update` prend un `ComponentManager&` et pas une `EntityList`** : un système a besoin d'accéder aux données typées (`getArray<Component>().getDense()`), pas à une liste brute d'IDs. `ComponentManager` est le seul point d'accès pertinent.

**Pourquoi `createSystem` renvoie `SystemList::iterator` et pas `std::unique_ptr<ISystem>` ni `ISystem*`** :
- Renvoyer `unique_ptr<ISystem>` transférerait la propriété à l'appelant — mais c'est `mSystems` qui doit rester seul propriétaire, sinon le système ne serait plus exécuté par `process()`. Deux `unique_ptr` ne peuvent pas posséder le même objet (double-free garanti dès que l'un des deux se détruit).
- Un pointeur brut marcherait (stable tant qu'on n'utilise pas `std::vector` pour `mSystems` — un `vector` invaliderait les pointeurs vers les *cases* à chaque réallocation ou `erase`), mais un itérateur de `plf::hive` offre la même stabilité tout en restant dans le style du reste du code (cf. `mSparse` qui utilise déjà des itérateurs de `hive` comme handles).

**Pourquoi `SystemList = plf::hive<...>` et pas `std::vector<...>`** : un itérateur de `std::vector` est invalidé par toute réallocation (insertion au-delà de la capacité) et par tout `erase` (qui décale les éléments suivants). `plf::hive` garantit qu'un itérateur reste valide tant que l'élément visé n'est pas lui-même supprimé, peu importe ce qui se passe ailleurs dans la structure — exactement ce qu'il faut pour que le handle rendu par `createSystem` reste utilisable plus tard par `destroySystem`, même après d'autres créations/suppressions entre-temps.

### Registry (façade)

```cpp
class Registry {
public:
    void update();  // sm.process(cm)

    Entity createEntity();
    void destroyEntity(Entity entity);

    template<typename Component>
    Component &addComponent(Entity entity, Component component);

    template<typename Component>
    void removeComponent(Entity entity);

    template<typename Component>
    Component &getComponent(Entity entity);

    template<typename Component>
    bool hasComponent(Entity entity);

    template<typename Component>
    ComponentArray<Component> &getArray();

    template<typename Component>
    SystemList::iterator createSystem(std::function<void(Component &)> func);

    void destroySystem(SystemList::iterator system);

private:
    ComponentManager mComponentManager;  // doit être déclaré AVANT mEntityManager, voir note
    EntityManager mEntityManager;
    SystemManager mSystemManager;
};
```

**Pourquoi `mComponentManager` est déclaré avant `mEntityManager`** : en C++, les membres sont construits dans leur ordre de déclaration, pas dans l'ordre de la liste d'initialisation du constructeur. Si `EntityManager` garde une référence vers `ComponentManager` (pour pouvoir nettoyer les composants d'une entité détruite), `ComponentManager` doit être construit en premier — sinon la référence se lierait à un objet pas encore construit (comportement indéfini).

**Pourquoi `Registry` ne fait que relayer** : c'est la seule classe publique que le reste du moteur/jeu doit connaître. `EntityManager`/`ComponentManager`/`SystemManager` restent des détails d'implémentation — aucun code extérieur ne devrait les manipuler directement.

### Exemple d'assemblage (déclarations uniquement)

```cpp
struct Velocity2d { int vx; int vy; };

Registry registry;

Entity e = registry.createEntity();
registry.addComponent<Position2d>(e, Position2d{0, 0});
registry.addComponent<Velocity2d>(e, Velocity2d{1, 0});

registry.createSystem<Position2d>([](Position2d &pos) {
    // logique du système — pas de corps ici, juste l'idée
});

// boucle de jeu :
registry.update();
```

---

## Partie 2 — Extension future : enable/disable par entité (non implémentée)

**Prérequis avant implémentation : un besoin concret doit être identifié.** Cette partie ne doit pas être construite par anticipation — c'est le genre de fonctionnalité qui se conçoit mieux une fois qu'on sait précisément ce qu'elle doit faire (freeze d'un ennemi, cutscene, transition de mort...), plutôt qu'en devinant à l'avance. Une alternative plus simple existe pour "mettre en pause" une entité et couvre déjà une bonne partie des cas réels : retirer temporairement le composant concerné (`removeComponent`/`addComponent`) plutôt que d'implémenter ce qui suit.

Si le besoin se confirme, voici le design retenu pendant la conception :

### Deux niveaux de désactivation

```cpp
struct Disabled {};  // désactive TOUS les systèmes pour cette entité

template<typename Component>
struct DisabledSystem;  // déclarée, JAMAIS définie en général — incomplète par défaut

// Spécialisation : uniquement pour les composants qui ont un système désactivable
template<>
struct DisabledSystem<Gravity2d> {};
```

**Pourquoi laisser `DisabledSystem<Component>` non définie par défaut** : ça transforme "ce composant n'a pas de système désactivable" en erreur de compilation plutôt qu'en bug silencieux à l'exécution. `cm.addComponent<DisabledSystem<Velocity2d>>(entity)` ne compile pas tant que `DisabledSystem<Velocity2d>` n'a pas été explicitement spécialisée — le type est incomplet, donc impossible à stocker dans un `ComponentArray`.

### System\<Component, D\>

```cpp
template<typename Component, typename D>
class System : public ISystem {
public:
    explicit System(std::function<void(Component &)> func);
    void update(ComponentManager &cm) override;

private:
    std::function<void(Component &)> mFunc;
};
```

`D` est le tag spécifique à ce système (typiquement `DisabledSystem<Component>`). `update` doit vérifier, pour chaque entité itérée :
1. `hasComponent<Disabled>(entity)` — désactivation globale,
2. `hasComponent<D>(entity)` — désactivation de ce système précis.

**Pourquoi centraliser la vérification de `Disabled` sur `ISystem`** : `System<Component>` (sans `D`) doit *aussi* respecter `Disabled` — sinon `disable(entity)` (le global) n'aurait aucun effet sur les systèmes créés sans tag spécifique. Pour ne pas dupliquer/oublier cette vérification dans chaque variante :

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

### API sur Registry / SystemManager

```cpp
template<typename Component, typename D>
SystemList::iterator createSystem(std::function<void(Component &)> func);

template<typename Component>
void enable(Entity entity);   // cm.removeComponent<DisabledSystem<Component>>(entity)
template<typename Component>
void disable(Entity entity);  // cm.addComponent<DisabledSystem<Component>>(entity, {})
template<typename Component>
bool isEnabled(Entity entity) const;

void enable(Entity entity);   // cm.removeComponent<Disabled>(entity)
void disable(Entity entity);  // cm.addComponent<Disabled>(entity, {})
bool isEnabled(Entity entity) const;
```

### Limite connue à garder en tête

`enable<Component>`/`disable<Component>` ne distinguent que par **type de composant**, pas par instance de système. S'il existe un jour deux systèmes différents qui lisent tous les deux `Gravity2d` (ex: un pour la physique, un pour du debug visuel), les désactiver indépendamment n'est **pas** possible avec cette API telle quelle — `disable<Gravity2d>` les couperait tous les deux en même temps. Tant qu'il n'y a qu'un seul système "propriétaire" par type de composant, ce n'est pas un problème. Si ce cas se présente, il faudra indexer par handle de système (`SystemList::iterator`) plutôt que par type.

### Note de performance si implémenté

Dans `System<Component, D>::update`, récupérer `getArray<Disabled>()` et `getArray<DisabledSystem<Component>>()` **une seule fois avant la boucle**, pas à chaque entité — sinon chaque `hasComponent<T>` refait un lookup dans la table `type_index` de `ComponentManager` à chaque itération, pour chaque système, à chaque frame.
