# Guide d'utilisation de l'ECS

> 🌐 **Langue :** Français · [English](ECS_GUIDE.en.md)

Ce guide explique comment **utiliser** l'ECS du moteur (`Engine/System`) au quotidien : créer des entités, leur attacher des composants, écrire des systèmes et, surtout, **éviter les pièges les plus courants**.

- **Public visé :** toute personne qui écrit du code de jeu au-dessus du moteur. Aucune connaissance préalable des ECS n'est nécessaire.
- **Pour comprendre les choix d'architecture** (le *pourquoi*), consultez [les notes de conception de l'ECS](ECS_DESIGN.fr.md).

Toute l'API passe par une seule classe : **`RType::GameManager`**. Les classes internes (`EntityManager`, `ComponentManager`, `SystemManager`) sont des détails d'implémentation : le code de jeu ne doit jamais les utiliser directement.

> [!TIP]
> Chaque règle de ce guide est couverte par un test du projet Poc (`Games/Poc`). En cas de doute sur un comportement, lancez le binaire `r-type_poc` ou lisez le test correspondant.

---

## Sommaire

1. [L'essentiel en 30 secondes](#1-lessentiel-en-30-secondes)
2. [Les entités](#2-les-entités)
3. [Les composants](#3-les-composants)
4. [Les systèmes](#4-les-systèmes)
5. [La frame : opérations immédiates et différées](#5-la-frame--opérations-immédiates-et-différées)
6. [La gestion des erreurs](#6-la-gestion-des-erreurs)
7. [Les pièges à éviter](#7-les-pièges-à-éviter)
8. [Recettes](#8-recettes)
9. [Aide-mémoire de l'API](#9-aide-mémoire-de-lapi)

### Vocabulaire

| Terme | Définition |
|---|---|
| **Entité** (`Entity`) | Un identifiant qui représente un objet du jeu (vaisseau, balle, ennemi…). Elle ne contient aucune donnée. |
| **Composant** | Une struct de données attachée à une entité (position, vie…). Une entité a au plus un composant de chaque type. |
| **Système** | Une fonction appelée à chaque `update()` : une fois par entité qui possède tous les composants demandés, ou une seule fois si aucun composant n'est demandé (système par frame). C'est là que vit la logique du jeu. |
| **Vue** | Un objet qui parcourt les entités possédant un ensemble de composants, et donne pour chacune l'entité et ses composants. |
| **Frame** | Un appel à `gm.update()` : tous les systèmes s'exécutent une fois, dans l'ordre. |
| **Handle** | La valeur `Entity` que vous conservez pour désigner une entité plus tard. |

---

## 1. L'essentiel en 30 secondes

```cpp
#include "GameManager.hpp"

struct Body { float x; float y; float vx; float vy; };
struct Health { int hp; };

RType::GameManager gm;

RType::Entity ship = gm.createEntity();
gm.addComponent<Body>(ship, 10.f, 50.f, 1.f, 0.f);
gm.addComponent<Health>(ship, 3);

gm.createSystem<Body>([](Body &b) {
    b.x += b.vx;
    b.y += b.vy;
});

while (running) {
    gm.update();
}
```

Trois idées à retenir :

- **Une entité n'est qu'un identifiant.** Elle ne contient rien : ce sont ses composants qui portent les données.
- **Un composant est une simple struct de données.** Pas d'héritage, pas de logique.
- **Un système est une fonction appelée sur chaque entité qui possède les composants demandés**, une fois par `update()`.

---

## 2. Les entités

### Créer, détruire, vérifier

```cpp
RType::Entity e = gm.createEntity();   // toujours immédiat
gm.destroyEntity(e);                   // détruit l'entité ET tous ses composants
bool alive = gm.hasEntity(e);          // false une fois l'entité détruite
```

### Des handles générationnels

Quand une entité est détruite, son emplacement (son *index*) est **recyclé** pour une future entité. Pour qu'un ancien handle ne puisse jamais désigner la nouvelle entité, chaque `Entity` porte aussi une **génération**, incrémentée à chaque recyclage :

```cpp
RType::Entity enemy = gm.createEntity();   // index 3, génération 0
gm.destroyEntity(enemy);

RType::Entity bullet = gm.createEntity();  // index 3, génération 1

gm.hasEntity(enemy);             // false : génération périmée
gm.hasComponent<Health>(enemy);  // false
gm.getComponent<Health>(enemy);  // lance NoSuchEntity
enemy == bullet;                 // false : même index, génération différente
```

Un handle périmé est donc **inoffensif** : il ne touchera jamais l'entité qui a récupéré son emplacement. C'est ce qui permet de stocker dans un composant l'`Entity` d'une **autre** entité (une cible, un enfant…) sans risque.

### Ce que permet le type `Entity`

| Opération | Disponible |
|---|---|
| Copier, comparer avec `==` | Oui |
| `e.index()`, `e.generation()` | Oui (pour le débogage, ou pour indexer votre propre tableau) |
| Constructeur par défaut `Entity e;` | **Non** : une `Entity` provient toujours de `createEntity()` |
| `std::set<Entity>`, `std::map<Entity, …>` | **Non** (pas d'`operator<`) |
| `std::unordered_map<Entity, …>` | **Non** (pas de `std::hash`) |

Pour conserver un ensemble d'entités, un `std::vector<Entity>` associé à `std::ranges::find` / `std::erase` suffit.

> [!WARNING]
> **Ne fabriquez jamais une `Entity` à la main** (`RType::Entity(3)`, `RType::Entity(3, 2)`). Le constructeur est public pour des raisons techniques, mais un handle fabriqué peut désigner une entité qui ne vous appartient pas dès que sa génération coïncide. Une `Entity` s'obtient uniquement avec `createEntity()`, ou depuis une vue.

---

## 3. Les composants

### Écrire un composant

Un composant est une struct de **données pures** :

```cpp
struct Health {
    int hp;
};

struct Player {};   // composant « tag » : vide, il sert uniquement de marqueur

struct Target {
    RType::Entity entity;   // une AUTRE entité : on stocke son handle
};
```

- Un composant doit être **déplaçable** (*move*). Les types non copiables (contenant un `std::unique_ptr`, par exemple) sont acceptés.
- Un composant n'a **pas besoin de connaître sa propre entité** : quand un système en a besoin, une vue la lui fournit (voir la [section 4](#accéder-à-lentité-depuis-un-système)). Ne stockez une `Entity` que pour désigner une *autre* entité.

### Ajouter, lire, retirer

```cpp
gm.addComponent<Position>(e, 10.f, 20.f);  // les arguments sont transmis au constructeur
gm.addComponent<Health>(e);                // sans argument : tous les champs à zéro
gm.addComponent<Player>(e);                // un tag

Position &pos = gm.getComponent<Position>(e);
pos.x += 1.f;                              // modifie directement le composant stocké

if (gm.hasComponent<Player>(e)) { /* ... */ }

gm.removeComponent<Health>(e);             // retire un composant
gm.removeComponents(e);                    // retire TOUS les composants ; l'entité reste vivante
```

- `addComponent` **ne renvoie rien**. Si vous avez besoin du composant juste après, utilisez `getComponent` (hors `update()`, l'ajout est immédiat : voir la [section 5](#5-la-frame--opérations-immédiates-et-différées)).
- Les arguments sont transmis au constructeur **entre parenthèses**, ce qui fonctionne aussi avec les struct simples : `Position{10.f, 20.f}` et `addComponent<Position>(e, 10.f, 20.f)` donnent le même résultat.
- Une entité possède **au plus un composant de chaque type**. Ajouter un `Health` à une entité qui en a déjà un lance `AlreadyExistingComponent`.

### Parcourir des entités : les vues

Pour parcourir les entités qui possèdent un ou plusieurs types de composants, utilisez une **vue** :

```cpp
gm.view<Position, Velocity>().each(
    [](RType::Entity e, Position &pos, const Velocity &vel) {
        pos.x += vel.vx;
        pos.y += vel.vy;
    });
```

- La fonction est appelée pour chaque entité qui possède **tous** les types demandés. Elle reçoit **l'entité en premier**, puis une référence vers chaque composant, dans l'ordre des types. Un paramètre peut être `const` si le composant n'est que lu.
- Une vue sur un type qui n'a **jamais été ajouté** est simplement vide : elle ne lance jamais d'exception.
- La vue parcourt automatiquement le type qui a **le moins de composants** : l'ordre des types ne change pas les performances.
- Une vue ne copie rien et ne garde aucune liste d'entités : la créer ne coûte presque rien. Créez-en une au moment de vous en servir, plutôt que de la conserver.
- Un même type ne peut pas apparaître deux fois : `gm.view<Position, Position>()` ne compile pas.
- Hors `update()`, ne détruisez pas d'entités pendant un `each` (voir la [section 7, piège n°3](#piège-n3--détruire-des-entités-pendant-un-parcours-hors-update)).

> [!NOTE]
> L'ordre de parcours **ne correspond pas à l'ordre d'ajout** : les emplacements libérés sont réutilisés. Ne vous reposez jamais sur un ordre particulier.

Dans un système, la vue se parcourt aussi avec une boucle `for`, grâce à `getVector()` : voir la [section 4](#parcourir-une-vue-dans-un-système--getvector).

Pour compter les composants d'un type : `gm.getArray<T>().nbComponent()`. `getArray<T>()` donne aussi un accès bas niveau au stockage (`getDense()`, dont chaque élément expose `.component` et `.entity()`), mais lance `NoSuchComponent` si aucun `T` n'a jamais été ajouté : pour parcourir, préférez toujours une vue.

### Les références vers un composant

Une référence obtenue avec `getComponent` (ou pendant un parcours) **reste valide lorsque d'autres composants sont ajoutés ou retirés**. Elle devient invalide uniquement lorsque **ce composant précis** est retiré (ou que son entité est détruite).

```cpp
Position &pos = gm.getComponent<Position>(player);
for (int i = 0; i < 1000; i++)
    gm.addComponent<Position>(gm.createEntity(), 0.f, 0.f);
pos.x = 5.f;                         // OK : pos est toujours valide

gm.destroyEntity(player);
pos.x = 6.f;                         // INTERDIT : le composant n'existe plus
```

---

## 4. Les systèmes

### Créer un système

```cpp
RType::SystemId gravity = gm.createSystem<Velocity>([](Velocity &v) {
    v.y += 0.5f;
});
```

- Le type du composant **doit être précisé** entre chevrons : `createSystem<Velocity>(...)`.
- À chaque `gm.update()`, la fonction est appelée **une fois pour chaque `Velocity` existant**.
- Les systèmes s'exécutent **dans leur ordre de création**. Un système voit les modifications faites par les systèmes précédents au cours de la même frame.
- Tant qu'un des types demandés n'a jamais été ajouté, le système ne fait simplement rien, sans erreur.

### Un système sur plusieurs composants

Un système peut demander plusieurs types de composants. La fonction reçoit alors une référence vers chacun, dans le même ordre, et n'est appelée que pour les entités qui les possèdent **tous** :

```cpp
gm.createSystem<Position, Velocity>([](Position &pos, const Velocity &vel) {
    pos.x += vel.vx;
    pos.y += vel.vy;
});
```

- Une entité qui ne possède qu'une partie des types est ignorée, sans erreur.
- Un paramètre peut être `const` si le système ne fait que lire ce composant.
- Un même type ne peut pas apparaître deux fois : `createSystem<Position, Position>` ne compile pas (*each component type must appear only once*).
- L'ordre des types ne change pas les performances : le système parcourt automatiquement le type qui a **le moins de composants**.

### Recevoir le `GameManager`

Un système qui doit agir sur le monde (créer ou détruire des entités, lire un autre composant…) **reçoit le `GameManager` en premier paramètre** :

```cpp
gm.createSystem<Homing, Position>(
    [](RType::GameManager &gm, Homing &h, Position &pos) {
        if (!gm.hasEntity(h.target))
            return;
        const Position &goal = gm.getComponent<Position>(h.target);
        pos.x += (goal.x - pos.x) * 0.1f;
    });
```

Les deux formes, avec et sans `GameManager`, peuvent être mélangées librement. Ne capturez pas le `GameManager` dans la lambda d'un système (`[&gm]`) : le recevoir en paramètre permet d'écrire le système comme une **fonction ordinaire**, dans un fichier à part (voir ci-dessous). À l'intérieur d'un système, en revanche, la lambda passée à `each` peut capturer le `gm` reçu : elle ne vit que le temps du parcours. Avec une boucle sur `getVector()`, il n'y a même pas de lambda : `gm` est directement utilisable.

### Un système par frame

Un système créé **sans aucun type de composant** est appelé **une seule fois par `update()`**. Il reçoit le `GameManager`, ou rien :

```cpp
gm.createSystem([](RType::GameManager &gm) {   // équivalent : gm.createSystem<>(...)
    for (auto &[e, enemy, pos] : gm.view<Enemy, Position>().getVector()) {
        if (pos.x < 0.f)
            gm.destroyEntity(e);               // différé : sans danger pendant la boucle
    }
});
```

À l'intérieur, les entités se parcourent avec une **vue** (voir [ci-dessous](#parcourir-une-vue-dans-un-système--getvector)).

C'est l'endroit naturel pour tout ce qui ne concerne pas une seule entité : les **collisions** entre deux groupes d'entités (voir la [recette](#des-collisions-entre-balles-et-ennemis)), l'apparition des vagues d'ennemis, le score, un chronomètre…

Un système par frame se comporte comme les autres : il a un `SystemId`, s'exécute à son rang dans l'ordre de création, et ses modifications structurelles sont différées à la fin de la frame.

### Parcourir une vue dans un système : `getVector()`

Dans un système, le plus lisible est de parcourir une vue avec une boucle `for` classique. `getVector()` transforme la vue en un `std::vector` qui contient, pour chaque entité, un tuple avec **l'entité puis une référence vers chacun de ses composants** :

```cpp
inline void bounds(RType::GameManager &gm)
{
    for (auto &[e, pos, vel] : gm.view<Position, Velocity>().getVector()) {
        pos.x += vel.vx;
        if (pos.x > 1920.f)
            gm.destroyEntity(e);   // différé : sans danger pendant la boucle
    }
}

gm.createSystem(bounds);
```

- Le type exact est `std::vector<std::tuple<RType::Entity, Position &, Velocity &>>` : les composants sont des **références**, et une modification dans la boucle modifie bien le vrai composant.
- La boucle permet `break`, `continue` et `return`, et la liste peut être parcourue plusieurs fois : c'est ce qui rend les **boucles imbriquées** (collisions) simples à écrire (voir la [recette](#des-collisions-entre-balles-et-ennemis)).
- Le vector est une **photo** prise au moment de l'appel : une entité ou un composant ajouté ensuite n'y apparaît pas. Appelez `getVector()` juste avant la boucle, et ne le conservez pas d'une frame à l'autre.
- Il **alloue** un vector à chaque appel. `each` (voir [les vues](#parcourir-des-entités--les-vues)) n'alloue rien, et reste disponible : `gm.view<A, B>().each([&gm](RType::Entity e, A &a, B &b) {...})`.

**Lequel choisir ?**

| Besoin | À utiliser |
|---|---|
| Un parcours dans un système, cas général | `getVector()` |
| Sortir de la boucle (`break`, `return`) | `getVector()` |
| Une boucle imbriquée (collisions) : construire la liste intérieure une seule fois | `getVector()` |
| Détruire des entités pendant le parcours, hors `update()` | `getVector()` (voir le [piège n°3](#piège-n3--détruire-des-entités-pendant-un-parcours-hors-update)) |
| Un parcours simple sur un très grand nombre d'entités, sans allocation | `each` |

### Accéder à l'entité depuis un système

La fonction d'un système par entité reçoit les composants, **pas l'entité**. Quand un système a besoin de l'entité (pour la détruire, lui ajouter ou retirer un composant…), écrivez un **système par frame qui parcourt une vue** : la vue fournit l'entité.

```cpp
struct Weapon {
    int ammo;   // pas besoin de stocker l'entité
};

gm.createSystem([](RType::GameManager &gm) {
    for (auto &[ship, w, from] : gm.view<Weapon, Position>().getVector()) {
        RType::Entity bullet = gm.createEntity();
        gm.addComponent<Position>(bullet, from.x, from.y);
        gm.addComponent<Velocity>(bullet, 5.f, 0.f);
        if (--w.ammo <= 0)
            gm.removeComponent<Weapon>(ship);   // plus de munitions : on retire l'arme
    }
});
```

`ship`, `w` et `from` sont l'entité et des **références** vers ses composants : `--w.ammo` modifie bien le vrai `Weapon`. La même chose avec `each` :

```cpp
gm.view<Weapon, Position>().each(
    [&gm](RType::Entity ship, Weapon &w, const Position &from) { /* ... */ });
```

Ne recopiez pas l'entité dans un champ du composant (`owner`, `self`) : il faudrait la renseigner à la main à chaque `addComponent`, et une erreur passerait inaperçue.

### Écrire ses systèmes dans des fichiers

Un système peut être une lambda, mais aussi une **fonction ordinaire**, ce qui permet de regrouper la logique du jeu dans des fichiers dédiés :

```cpp
// Systems.hpp
inline void move(Position &pos, const Velocity &vel)
{
    pos.x += vel.vx;
    pos.y += vel.vy;
}

inline void collisions(RType::GameManager &gm)
{
    for (auto &[bullet, b, pos] : gm.view<Bullet, Position>().getVector()) {
        /* ... */
    }
}

// Au démarrage
gm.createSystem<Position, Velocity>(move);
gm.createSystem(collisions);
```

Une fonction **définie** dans un header doit être `inline` ; sinon, déclarez-la dans le `.hpp` et définissez-la dans un `.cpp`.

Lorsque plusieurs données vont toujours ensemble, regroupez-les dans un seul composant (comme `Body { x, y, vx, vy }` dans l'exemple d'introduction) : un seul composant, un seul système, aucune recherche.

### Détruire un système

```cpp
gm.destroySystem(gravity);
gm.hasSystem(gravity);   // false
```

Un `SystemId` n'est **jamais réutilisé** : un ancien identifiant ne pourra jamais détruire un autre système par erreur. Détruire un système qui n'existe plus n'a aucun effet.

> [!IMPORTANT]
> **Créez vos systèmes une seule fois, au démarrage, et conservez-les.** Pour activer ou désactiver un comportement, utilisez des données dans les composants (un `bool enabled`, un tag que vous ajoutez ou retirez) plutôt que de créer et détruire des systèmes en cours de partie. Voir la [section 7](#7-les-pièges-à-éviter).

---

## 5. La frame : opérations immédiates et différées

C'est **la** notion essentielle de cet ECS.

Pendant un `update()`, les systèmes parcourent les tableaux de composants. Si un système détruisait une entité en plein parcours, il corromprait le tableau en cours de lecture. Pour l'éviter, **les modifications structurelles demandées pendant `update()` sont mises en attente** dans une file de commandes, puis appliquées **toutes ensemble à la fin de la frame**, dans l'ordre où elles ont été demandées.

En dehors d'`update()`, tout est immédiat.

| Opération | Hors `update()` | Pendant `update()` (dans un système) |
|---|---|---|
| `createEntity` | Immédiat | Immédiat |
| `destroyEntity` | Immédiat | **Fin de frame** |
| `addComponent` | Immédiat | **Fin de frame** |
| `removeComponent` | Immédiat | **Fin de frame** |
| `removeComponents` | Immédiat | **Fin de frame** |
| `createSystem` | Immédiat | Immédiat (non supporté : voir la [section 7](#7-les-pièges-à-éviter)) |
| `destroySystem` | Immédiat | **Fin de frame** |
| `getComponent`, `hasComponent`, `hasEntity`, `hasSystem`, `view` | Lecture | Lecture de l'état **avant** application des commandes en attente |

### Conséquences concrètes

```cpp
struct Life {
    int hp;
};

gm.createSystem([](RType::GameManager &gm) {
    gm.view<Life>().each([&gm](RType::Entity e, Life &l) {
        if (l.hp > 0)
            return;
        gm.destroyEntity(e);

        gm.hasEntity(e);                 // encore TRUE : la destruction est en attente
        gm.getComponent<Position>(e);    // fonctionne encore, jusqu'à la fin de la frame
    });
});
```

- Une entité détruite pendant la frame **reste visible et lisible jusqu'à la fin de la frame**. Les systèmes suivants la voient encore.
- Un composant ajouté pendant la frame **n'apparaît qu'à la fin de celle-ci** : `hasComponent` renvoie `false` d'ici là, et les systèmes et les vues ne le parcourent qu'à partir de la frame suivante.
- Une entité créée pendant la frame existe immédiatement (`createEntity` est immédiat), mais ses composants ne sont ajoutés qu'en fin de frame.
- Si deux systèmes détruisent la même entité au cours de la même frame (deux balles touchent le même ennemi), il n'y a aucun danger : la seconde destruction n'a aucun effet.
- Une entité détruite pendant la frame n'est **pas** recyclée dans la même frame : `createEntity` ne réutilisera jamais son emplacement avant la fin de la frame.
- Appeler `gm.update()` depuis un système n'a aucun effet (pas de frame imbriquée).

---

## 6. La gestion des erreurs

Toutes les exceptions de l'ECS héritent de `RType::EngineException` (déclarée dans `Exceptions.hpp`).

| Appel | Situation | Résultat |
|---|---|---|
| `destroyEntity(e)` | `e` déjà détruite ou périmée | Aucun effet |
| `hasComponent<T>(e)` | `e` détruite ou périmée | `false` (ne lance jamais d'exception) |
| `getComponent<T>(e)` | `e` détruite ou périmée | `NoSuchEntity` |
| | `e` n'a pas de `T` | `NoSuchComponent` |
| `addComponent<T>(e, ...)` | `e` détruite ou périmée | `NoSuchEntity` |
| | `e` possède déjà un `T` | `AlreadyExistingComponent` |
| | le constructeur de `T` lance une exception | l'exception du constructeur, **immédiatement** |
| `removeComponent<T>(e)` | `e` détruite ou périmée | `NoSuchEntity` |
| | `e` n'a pas de `T` | `NoSuchComponent` |
| `removeComponents(e)` | `e` détruite ou périmée | `NoSuchEntity` |
| `view<Ts...>()` | un des `Ts` n'a jamais existé | Vue vide (ne lance jamais d'exception) |
| `getArray<T>()` | aucun `T` n'a jamais existé | `NoSuchComponent` |
| `destroySystem(id)` | système déjà détruit | Aucun effet |

### Les erreurs pendant `update()`

Pendant `update()`, une commande en attente ne s'exécute **qu'en fin de frame**. Son erreur ne sort donc **pas** de l'appel fait dans le système, mais de `gm.update()` :

```cpp
struct Bullet {
    RType::Entity target;
};

gm.createSystem<Bullet>([](RType::GameManager &gm, Bullet &b) {
    try {
        gm.addComponent<Damage>(b.target, 1);   // b.target est peut-être déjà détruite...
    } catch (const RType::NoSuchEntity &) {
        // ... mais ce catch ne sera JAMAIS atteint
    }
});

try {
    gm.update();                                // c'est ICI que NoSuchEntity est levée
} catch (const RType::EngineException &e) {
    std::cerr << e.what() << std::endl;
}
```

La bonne pratique consiste à **vérifier avant d'agir** plutôt qu'à rattraper l'erreur après coup :

```cpp
gm.createSystem<Bullet>([](RType::GameManager &gm, Bullet &b) {
    if (gm.hasEntity(b.target) && !gm.hasComponent<Damage>(b.target))
        gm.addComponent<Damage>(b.target, 1);
});
```

### Les garanties d'`update()` en cas d'exception

- **La frame est toujours appliquée.** Même si un système lance une exception, toutes les commandes déjà demandées pendant la frame sont exécutées avant que l'exception ne sorte d'`update()`.
- **Une commande en échec n'empêche pas les suivantes.** Toutes les commandes de la file sont exécutées.
- **Une seule erreur est remontée.** Si un système a lancé une exception, c'est elle qui sort d'`update()` (les erreurs des commandes sont alors ignorées). Sinon, c'est l'erreur de la première commande en échec. Les autres erreurs sont perdues : d'où l'intérêt de vérifier avant d'agir.
- **L'ECS reste utilisable ensuite.** L'`update()` suivant fonctionne normalement ; rien n'est rejoué.

---

## 7. Les pièges à éviter

Chaque piège est présenté avec le code **à ne pas écrire**, puis la solution **recommandée**.

### Piège n°1 : appeler le `GameManager` depuis le constructeur ou le destructeur d'un composant

**À ne pas faire :**

```cpp
struct Link {
    RType::GameManager *gm;
    RType::Entity child;
    ~Link() { gm->destroyEntity(child); }   // INTERDIT
};
```

Le destructeur d'un composant est appelé **pendant** que l'ECS applique une commande (`destroyEntity`, `removeComponent`, fin de frame). Rappeler le `GameManager` à ce moment-là provoque un **dépassement de pile** (*stack overflow*) ou une corruption mémoire.

**À faire :** effectuez l'opération dans un système.

```cpp
struct Parent {
    RType::Entity child;
};

gm.createSystem([](RType::GameManager &gm) {
    gm.view<Parent, Health>().each(
        [&gm](RType::Entity self, Parent &p, const Health &h) {
            if (h.hp > 0)
                return;
            gm.destroyEntity(p.child);   // le parent meurt : l'enfant est détruit avec lui
            gm.destroyEntity(self);
        });
});
```

Le constructeur ou le destructeur d'un composant ne doit gérer **que ses propres ressources** (mémoire, handle de texture ou de son…).

### Piège n°2 : créer un système pendant `update()`

`createSystem` est immédiat, même dans un système. Le nouveau système est ajouté en fin de liste **pendant que celle-ci est parcourue** : il s'exécute donc dès la frame en cours. Un système qui en crée un autre à chaque passage fait boucler `update()` indéfiniment.

**À faire :** créez tous vos systèmes au démarrage. Pour un comportement temporaire, activez-le à l'aide d'une donnée dans un composant.

### Piège n°3 : détruire des entités pendant un parcours hors `update()`

Hors `update()`, `destroyEntity` est **immédiat** : il retire les composants du tableau en cours de parcours.

**À ne pas faire :**

```cpp
gm.view<Lifetime>().each([&gm](RType::Entity e, Lifetime &l) {
    if (l.ticks <= 0)
        gm.destroyEntity(e);                // INTERDIT : le parcours est corrompu
});
```

**À faire :** faites-le dans un système, où la destruction est automatiquement différée (voir la [recette](#une-durée-de-vie-limitée-balles-particules-explosions)). Si vous devez vraiment le faire hors `update()`, parcourez `getVector()` : la liste est construite avant la première destruction, donc détruire une entité ne la perturbe pas.

```cpp
for (auto &[e, l] : gm.view<Lifetime>().getVector())
    if (l.ticks <= 0)
        gm.destroyEntity(e);               // OK : le vector n'est pas touché
```

Une seule règle : hors `update()`, la destruction est immédiate, donc **ne touchez plus aux composants de `e` après l'avoir détruite** (ici `l`). Les références des autres entités du vector restent valides.

### Piège n°4 : conserver un pointeur ou une référence vers le composant d'une autre entité

**À ne pas faire :**

```cpp
struct Homing {
    Position *target;   // INTERDIT : pointeur invalide dès que la cible est détruite
};
```

**À faire :** conservez le **handle** de l'entité, et vérifiez qu'elle existe avant chaque accès.

```cpp
struct Homing {
    RType::Entity target;
};

gm.createSystem<Homing>([](RType::GameManager &gm, Homing &h) {
    if (!gm.hasEntity(h.target))
        return;   // la cible est détruite (ou son emplacement recyclé) : on cesse de la suivre
    Position &goal = gm.getComponent<Position>(h.target);
    // ...
});
```

Grâce aux générations, `hasEntity` reste fiable même si l'emplacement de la cible a été réutilisé par une autre entité.

### Piège n°5 : capturer par référence des variables à durée de vie courte

La fonction d'un système est **conservée tant que le système existe**. Tout ce qu'elle capture par référence doit vivre au moins aussi longtemps.

**À ne pas faire :**

```cpp
void setupScore(RType::GameManager &gm)
{
    int score = 0;
    gm.createSystem<Enemy>([&score](Enemy &) { score++; });
}   // score est détruit ici : le système écrit ensuite dans une zone mémoire libérée
```

**À faire :** stockez la donnée dans un composant, ou capturez un objet qui vit aussi longtemps que le `GameManager` (un membre de votre classe `Game`, par exemple). Le `GameManager` lui-même n'a jamais besoin d'être capturé : recevez-le en paramètre.

### Piège n°6 : compter sur un `try`/`catch` autour d'un appel dans un système

Voir la [section 6](#les-erreurs-pendant-update) : pendant `update()`, l'erreur d'un `addComponent`, `removeComponent` ou `removeComponents` sort de `gm.update()`, et non de l'appel lui-même.

**À faire :** vérifiez avec `hasEntity` / `hasComponent` avant d'agir.

### Piège n°7 : lire un composant juste après l'avoir ajouté, dans un système

**À ne pas faire :**

```cpp
gm.createSystem<Spawner>([](RType::GameManager &gm, Spawner &s) {
    RType::Entity e = gm.createEntity();
    gm.addComponent<Health>(e, 10);
    gm.getComponent<Health>(e);   // lance NoSuchComponent : l'ajout est en attente
});
```

**À faire :** conservez la valeur dans une variable locale si vous en avez besoin immédiatement. Le composant existera à la fin de la frame.

---

## 8. Recettes

### Une boucle de jeu type

```cpp
// Systems.hpp : un système = une fonction
inline void readInput(Input &in) { /* ... */ }
inline void move(Body &b) { /* ... */ }
inline void collisions(RType::GameManager &gm) { /* vues sur les balles et les ennemis */ }
inline void killDead(RType::GameManager &gm) { /* vue sur Health : destroyEntity si hp <= 0 */ }
inline void draw(const Body &b, const Sprite &s) { /* ... */ }

// main.cpp
RType::GameManager gm;

// 1. Au démarrage : tous les systèmes, dans l'ordre d'exécution souhaité.
gm.createSystem<Input>(readInput);
gm.createSystem<Body>(move);
gm.createSystem(collisions);
gm.createSystem(killDead);
gm.createSystem<Body, Sprite>(draw);

// 2. Les entités initiales.
spawnPlayer(gm);

// 3. La boucle principale.
while (running) {
    gm.update();
}
```

### Une durée de vie limitée (balles, particules, explosions)

```cpp
struct Lifetime {
    int ticks;
};

gm.createSystem([](RType::GameManager &gm) {
    gm.view<Lifetime>().each([&gm](RType::Entity e, Lifetime &l) {
        if (--l.ticks <= 0)
            gm.destroyEntity(e);   // différé : sans danger pendant le parcours
    });
});

RType::Entity bullet = gm.createEntity();
gm.addComponent<Lifetime>(bullet, 60);
```

### Mettre une entité en pause sans détruire de système

```cpp
struct Frozen {};

gm.createSystem([](RType::GameManager &gm) {
    gm.view<Body>().each([&gm](RType::Entity e, Body &b) {
        if (gm.hasComponent<Frozen>(e))
            return;
        b.x += b.vx;
        b.y += b.vy;
    });
});

gm.addComponent<Frozen>(enemy);      // pause
gm.removeComponent<Frozen>(enemy);   // reprise
```

### Un projectile qui vise une cible susceptible d'être détruite

```cpp
struct Missile {
    RType::Entity target;   // une AUTRE entité : on stocke son handle
};

gm.createSystem([](RType::GameManager &gm) {
    gm.view<Missile, Position>().each(
        [&gm](RType::Entity self, Missile &m, Position &pos) {
            if (!gm.hasEntity(m.target)) {
                gm.destroyEntity(self);   // plus de cible : le missile s'autodétruit
                return;
            }
            const Position &goal = gm.getComponent<Position>(m.target);
            pos.x += (goal.x - pos.x) * 0.1f;
            pos.y += (goal.y - pos.y) * 0.1f;
        });
});
```

### Des collisions entre balles et ennemis

Un système par frame, avec deux boucles imbriquées : chaque balle est testée contre chaque ennemi.

```cpp
inline void collisions(RType::GameManager &gm)
{
    auto enemies = gm.view<Enemy, Position, Health>().getVector();   // construite une seule fois

    for (auto &[bullet, b, bp, d] : gm.view<Bullet, Position, Damage>().getVector()) {
        for (auto &[enemy, en, ep, h] : enemies) {
            if (!overlap(bp, ep))
                continue;
            h.hp -= d.value;
            gm.destroyEntity(bullet);   // différé : la balle reste visible jusqu'à la fin de la frame
            break;                      // une balle ne touche qu'un ennemi
        }
    }
}

gm.createSystem(collisions);
```

- La liste des ennemis est construite **une fois par frame**, et non une fois par balle comme le ferait une vue imbriquée.
- Le `break` empêche une balle de toucher deux ennemis dans la même frame : sans lui, comme sa destruction est différée, elle endommagerait tous les ennemis qu'elle chevauche.
- Le coût est proportionnel à *balles × ennemis*. Au-delà de quelques centaines d'entités, une grille spatiale construite une fois par frame, au début du système, devient nécessaire.

---

## 9. Aide-mémoire de l'API

```cpp
namespace RType {

class GameManager {
public:
    void update();

    // Entités
    Entity createEntity();
    void destroyEntity(Entity entity);
    bool hasEntity(Entity entity) const;

    // Composants
    template<typename Component, typename... Args>
    void addComponent(Entity entity, Args &&...args);
    template<typename Component>
    void removeComponent(Entity entity);
    void removeComponents(Entity entity);
    template<typename Component>
    Component &getComponent(Entity entity);
    template<typename Component>
    bool hasComponent(Entity entity) const;
    template<typename... Components>
    View<Components...> view();              // .each([](Entity, Components &...) {...})
                                             // .getVector() : std::vector<std::tuple<Entity, Components &...>>
    template<typename Component>
    ComponentArray<Component> &getArray();   // .nbComponent()

    // Systèmes (aucun type : une fois par frame)
    template<typename... Components>
    SystemId createSystem(std::function<void(Components &...)> func);
    template<typename... Components>
    SystemId createSystem(std::function<void(GameManager &, Components &...)> func);
    void destroySystem(SystemId id);
    bool hasSystem(SystemId id) const;
};

}
```

| Objectif | Appel |
|---|---|
| Créer une entité | `gm.createEntity()` |
| Savoir si une entité existe encore | `gm.hasEntity(e)` |
| Détruire une entité et ses composants | `gm.destroyEntity(e)` |
| Ajouter un composant | `gm.addComponent<T>(e, args...)` |
| Lire ou modifier un composant | `gm.getComponent<T>(e)` |
| Savoir si une entité possède un composant | `gm.hasComponent<T>(e)` |
| Retirer un composant | `gm.removeComponent<T>(e)` |
| Vider une entité sans la détruire | `gm.removeComponents(e)` |
| Parcourir des entités, avec leur entité | `gm.view<T, ...>().each(func)` |
| Parcourir avec une boucle `for` (`break`, boucles imbriquées) | `for (auto &[e, t, ...] : gm.view<T, ...>().getVector())` |
| Compter les composants d'un type | `gm.getArray<T>().nbComponent()` |
| Ajouter un comportement par entité | `gm.createSystem<T, ...>(func)` (au démarrage) |
| Ajouter un traitement global, une fois par frame | `gm.createSystem(func)` (au démarrage) |
| Donner le `GameManager` à un système | `GameManager &` en premier paramètre de `func` |
| Retirer un comportement | `gm.destroySystem(id)` |
| Faire avancer le jeu d'une frame | `gm.update()` |
