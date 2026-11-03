# Écrire un jeu sans `main` : `engine-game`

> 🌐 **Langue :** Français · [English](ENGINE_GAME.en.md)

`engine-game` est la bibliothèque qui fournit le `main` à la place du jeu. Le jeu n'écrit que ses fonctions d'initialisation et ses systèmes, puis les range dans une seule variable, `RType::Game`. Le moteur s'occupe du reste : créer le `GameManager`, la fenêtre, le clavier et l'horloge, appeler les fonctions du jeu dans l'ordre, puis faire tourner la boucle.

- **Public visé :** toute personne qui crée un jeu avec le moteur.
- **Pour l'ECS en détail** (entités, composants, systèmes, vues), consultez [le guide d'utilisation de l'ECS](ECS_GUIDE.fr.md).
- **Pour les composants fournis** (images, animations, texte, son), consultez [le guide de création d'un jeu](GAME_GUIDE.fr.md).

> [!TIP]
> Tous les exemples de ce guide ont été compilés et lancés avec le moteur actuel. Les messages d'erreur de la [section 8](#8-les-erreurs-et-comment-les-régler) ont été reproduits avec `g++-14`.

---

## Sommaire

1. [Ce que fait le `main` du moteur](#1-ce-que-fait-le-main-du-moteur)
2. [`GameInfo` : ce que le jeu fournit](#2-gameinfo--ce-que-le-jeu-fournit)
3. [Mettre en place le jeu](#3-mettre-en-place-le-jeu)
4. [Un jeu complet en un fichier](#4-un-jeu-complet-en-un-fichier)
5. [Écrire les inits](#5-écrire-les-inits)
6. [Le mode debug](#6-le-mode-debug)
7. [À faire / à ne pas faire](#7-à-faire--à-ne-pas-faire)
8. [Les erreurs et comment les régler](#8-les-erreurs-et-comment-les-régler)

---

## 1. Ce que fait le `main` du moteur

Le `main` se trouve dans [`Engine/Game/src/Main.cpp`](../Engine/Game/src/Main.cpp). Dans l'ordre, il :

1. crée le `GameManager` (en mode debug si le moteur est compilé avec `__DEBUG__`, voir [section 6](#6-le-mode-debug)) ;
2. crée trois entités, chacune avec **un seul** composant du moteur :
   - une `Window` aux dimensions et au titre donnés par `RType::Game` ;
   - un `KeyboardManager` ;
   - une `MasterClock` ;
3. appelle chaque fonction de `RType::Game.inits`, **dans l'ordre de la liste** ;
4. lance `gm.run(RType::Game.windowFPS, RType::Game.runningCondition)` : la boucle tourne tant que `runningCondition()` renvoie `true` ;
5. si une `RType::EngineException` remonte, affiche son message et quitte avec le code `84`.

Le `main` **crée les composants mais n'enregistre aucun système**. C'est le jeu qui enregistre, dans ses inits, les systèmes du moteur dont il a besoin (horloge, clavier, effacement de la fenêtre…), voir [section 5](#enregistrer-les-systèmes-du-moteur).

---

## 2. `GameInfo` : ce que le jeu fournit

Le header [`Game.hpp`](../Engine/Game/include/Game.hpp) déclare la structure et la variable :

```cpp
namespace RType {

    struct GameInfo {
        std::list<std::function<void(RType::GameManager &)>> inits = {};
        std::function<bool()> runningCondition;
        int windowFPS = 60;
        std::size_t windowWidth = 800;
        std::size_t windowHeight = 600;
        std::string windowName = "My Game";
    };

    extern GameInfo Game;

}
```

| Champ | Rôle | Par défaut |
|---|---|---|
| `inits` | Les fonctions appelées une fois au démarrage, dans l'ordre | vide |
| `runningCondition` | Appelée avant chaque frame : la boucle continue tant qu'elle renvoie `true` | **aucune, à fournir** |
| `windowFPS` | Le nombre de frames par seconde de la boucle | `60` |
| `windowWidth`, `windowHeight` | La taille de la fenêtre, en pixels | `800` × `600` |
| `windowName` | Le titre de la fenêtre | `"My Game"` |

Le jeu **définit** `RType::Game` dans **un seul** de ses `.cpp`, avec les designated initializers de C++20. Les champs non précisés gardent leur valeur par défaut :

```cpp
RType::GameInfo RType::Game = {
    .inits = {createWorld, createSystems},
    .runningCondition = [] { return !WindowShouldClose(); },
    .windowName = "Mon jeu",
};
```

Les champs doivent être écrits **dans l'ordre de la structure** : `inits`, `runningCondition`, `windowFPS`, `windowWidth`, `windowHeight`, `windowName`. On peut en sauter, pas les intervertir.

---

## 3. Mettre en place le jeu

### Le dossier

```
Games/
└── MonJeu/
    ├── CMakeLists.txt
    ├── include/
    │   └── MonJeu.hpp     # composants et déclarations des fonctions
    └── src/
        ├── Ball.cpp       # les fonctions qui créent les entités
        ├── Systems.cpp    # les systèmes
        └── Game.cpp       # uniquement la définition de RType::Game
```

Pas de `Main.cpp` : il est fourni par `engine-game`.

### Le `CMakeLists.txt`

```cmake
cmake_minimum_required(VERSION 3.25...4.0)
project(mon_jeu CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(mon_jeu
        src/Ball.cpp
        src/Systems.cpp
        src/Game.cpp
)
target_include_directories(mon_jeu PRIVATE include)
target_link_libraries(mon_jeu PRIVATE engine-game)
```

**`engine-game` suffit** : elle apporte `engine-system` et `engine-graphical`, avec leurs dossiers d'includes. Si le jeu n'a qu'un seul fichier et pas de dossier `include/`, la ligne `target_include_directories` est inutile.

Ajoutez ensuite le dossier dans [`Games/CMakeLists.txt`](../Games/CMakeLists.txt) :

```cmake
add_subdirectory(MonJeu)
```

et le nom du binaire dans le [`.gitignore`](../.gitignore), à côté de `r-type_client` et `r-type_server`.

### Compiler et lancer

```sh
cmake -S . -B build
cmake --build build --target mon_jeu -j
./Games/mon_jeu
```

Le binaire est placé directement dans `Games/`.

### Plusieurs fichiers

Les fonctions sont déclarées dans le header et définies dans leurs propres fichiers. `Game.cpp` ne contient que la définition de `RType::Game` :

```cpp
// include/Bounce.hpp
#include "GameManager.hpp"

namespace Bounce {

    struct Ball {
        float radius = 20.f;
    };

    void createBall(RType::GameManager &gm);
    void createSystems(RType::GameManager &gm);

} // namespace Bounce
```

```cpp
// src/Game.cpp
#include "Bounce.hpp"
#include "Game.hpp"
#include "Window.hpp"

RType::GameInfo RType::Game = {
    .inits = {Bounce::createBall, Bounce::createSystems},
    .runningCondition = [] { return !WindowShouldClose(); },
    .windowWidth = 1280,
    .windowHeight = 720,
    .windowName = "Bounce",
};
```

---

## 4. Un jeu complet en un fichier

Une balle qui rebondit sur les bords de la fenêtre. `Espace` inverse sa direction, `Échap` ou la croix ferment le jeu.

```cpp
#include <algorithm>
#include <cmath>

#include "Clock.hpp"
#include "Game.hpp"
#include "GameManager.hpp"
#include "KeyboardManager.hpp"
#include "Position2D.hpp"
#include "Velocity2D.hpp"
#include "Window.hpp"

namespace {

    constexpr float WIDTH = 800.f;
    constexpr float HEIGHT = 600.f;

    struct Ball {
        float radius = 20.f;
    };

    void createBall(RType::GameManager &gm)
    {
        RType::Entity ball = gm.createEntity();

        gm.addComponent<Ball>(ball);
        gm.addComponent<RType::Position2D>(ball, RType::Position2D {400.f, 300.f});
        gm.addComponent<RType::Velocity2D>(ball, RType::Velocity2D {250.f, 180.f});
    }

    void bounce(RType::GameManager &gm)
    {
        using namespace RType;

        for (auto [_, b, pos, vel] : gm.view<Ball, Position2D, Velocity2D>().getVector()) {
            if (pos.x < b.radius)
                vel.x = std::abs(vel.x);
            if (pos.x > WIDTH - b.radius)
                vel.x = -std::abs(vel.x);
            if (pos.y < b.radius)
                vel.y = std::abs(vel.y);
            if (pos.y > HEIGHT - b.radius)
                vel.y = -std::abs(vel.y);
        }
    }

    void reverseOnSpace(RType::GameManager &gm)
    {
        using namespace RType;

        for (auto [_, k] : gm.view<KeyboardManager>().getVector()) {
            if (std::ranges::find(k.keyPressed, Key::Value::Space) == k.keyPressed.end())
                continue;
            for (auto [_, b, vel] : gm.view<Ball, Velocity2D>().getVector()) {
                vel.x = -vel.x;
                vel.y = -vel.y;
            }
        }
    }

    void draw(RType::GameManager &gm)
    {
        using namespace RType;

        for (auto [_, b, pos] : gm.view<Ball, Position2D>().getVector())
            DrawCircleV({pos.x, pos.y}, b.radius, RED);
    }

    void createSystems(RType::GameManager &gm)
    {
        using namespace RType;

        gm.createSystem<MasterClock>(MasterClock::systemMasterClock);
        gm.createSystem<KeyboardManager>(KeyboardManager::systemKeyboardManager);
        gm.createSystem<>(reverseOnSpace);
        gm.createSystem<>(bounce);
        gm.createSystem<MasterClock>(Velocity2D::systemMasterVelocity2D);
        gm.createSystem<Window>(Window::systemClearWindow);
        gm.createSystem<>(draw);
    }

} // namespace

RType::GameInfo RType::Game = {
    .inits = {createBall, createSystems},
    .runningCondition = [] { return !WindowShouldClose(); },
    .windowName = "Bounce",
};
```

- **`createBall`** crée les entités du jeu, **`createSystems`** enregistre tous les systèmes, une seule fois.
- **`bounce`, `reverseOnSpace` et `draw`** prennent seulement le `GameManager` : enregistrées avec `gm.createSystem<>(...)`, elles sont appelées une fois par frame et trouvent leurs entités avec des vues.

---

## 5. Écrire les inits

### Une init est appelée une seule fois

Chaque fonction de `inits` est appelée **une fois**, au démarrage. Elle sert à créer des entités et à enregistrer des systèmes. Ce qui doit se passer **à chaque frame** (déplacements, logique du jeu, dessin) va dans un système :

```cpp
// bounce n'est appelée qu'une fois : la balle ne rebondit jamais
.inits = {createBall, bounce},

// bounce devient un système : elle est appelée à chaque frame
.inits = {createBall, [](RType::GameManager &gm) { gm.createSystem<>(bounce); }},
```

### Enregistrer les systèmes du moteur

Le `main` crée la `Window`, le `KeyboardManager` et la `MasterClock`, mais n'enregistre pas leurs systèmes. Sans eux, l'horloge reste à zéro, le clavier reste vide et la fenêtre ne se rafraîchit pas. Une init du jeu doit donc les enregistrer :

```cpp
gm.createSystem<RType::MasterClock>(RType::MasterClock::systemMasterClock);
gm.createSystem<RType::KeyboardManager>(RType::KeyboardManager::systemKeyboardManager);
gm.createSystem<RType::Window>(RType::Window::systemClearWindow);
```

Les systèmes s'exécutent **dans l'ordre où ils sont créés**, donc l'ordre des inits compte. L'ordre conseillé :

1. l'horloge et le clavier ;
2. la logique du jeu ;
3. la physique (`Velocity2D::systemMasterVelocity2D`) ;
4. `Window::systemClearWindow`, puis le dessin.

### Lire la fenêtre, le clavier et l'horloge

Ces trois composants sont **chacun sur sa propre entité**. Une vue qui demande plusieurs d'entre eux à la fois ne trouve donc aucune entité. Récupérez-les avec une vue chacun :

```cpp
void updateGame(RType::GameManager &gm)
{
    using namespace RType;

    auto keyboards = gm.view<KeyboardManager>().getVector();
    auto clocks = gm.view<MasterClock>().getVector();

    if (keyboards.empty() || clocks.empty())
        return;
    auto &k = std::get<1>(keyboards.front());
    auto &c = std::get<1>(clocks.front());
    float dt = float(c.getElapsed().count()) / MILLI_IN_SEC;
    // ...
}
```

### Une fonction qui prend d'autres paramètres

Les inits ont toutes la signature `void(RType::GameManager &)`. Pour une fonction qui prend d'autres paramètres, enveloppez-la dans une lambda qui les fixe :

```cpp
void createEnemy(RType::GameManager &gm, float x, float y);

RType::GameInfo RType::Game = {
    .inits = {
        createSystems,
        [](RType::GameManager &gm) { createEnemy(gm, 100.f, 50.f); },
        [](RType::GameManager &gm) { createEnemy(gm, 300.f, 50.f); },
    },
    .runningCondition = [] { return !WindowShouldClose(); },
};
```

### Arrêter le jeu

`runningCondition` est appelée avant chaque frame. `[] { return !WindowShouldClose(); }` arrête le jeu quand on ferme la fenêtre ou qu'on appuie sur `Échap`. Elle peut aussi lire un état global du jeu, par exemple un booléen passé à `true` par un système quand la partie est finie.

---

## 6. Le mode debug

Quand le moteur est compilé avec la macro `__DEBUG__`, le `main` crée le `GameManager` en mode debug : chaque opération est écrite sur `std::clog`.

```sh
cmake -S . -B build -DCMAKE_CXX_FLAGS="-D__DEBUG__"
cmake --build build --target mon_jeu -j
```

```
[GameManager] debug mode enabled
[GameManager] createEntity -> entity 0
[GameManager] addComponent<RType::Window> on entity 0
[GameManager] createEntity -> entity 1
[GameManager] addComponent<RType::KeyboardManager> on entity 1
[GameManager] createEntity -> entity 2
[GameManager] addComponent<RType::MasterClock> on entity 2
[GameManager] createSystem with ( RType::MasterClock ) -> system 0
```

La macro doit s'appliquer à `engine-game`, qui contient le `main` : un `target_compile_definitions` sur la cible du jeu ne suffit pas.

---

## 7. À faire / à ne pas faire

### À faire

- **Définir `RType::Game` dans un seul `.cpp`**, de préférence un `Game.cpp` qui ne contient que ça.
- **Toujours fournir `runningCondition`**.
- **Lier uniquement `engine-game`** dans le `CMakeLists.txt` du jeu.
- **Enregistrer l'horloge, le clavier et `Window::systemClearWindow`** dans une init.
- **Mettre la logique de chaque frame dans des systèmes**, enregistrés une fois dans une init.
- **Récupérer la fenêtre, le clavier et l'horloge avec une vue chacun**.
- **Écrire les champs de `GameInfo` dans l'ordre de la structure**.

### À ne pas faire

- **Ne pas écrire de `main`** dans le jeu : il remplacerait silencieusement celui du moteur.
- **Ne pas définir `RType::Game` dans un header**, ni dans deux `.cpp`.
- **Ne pas créer de `Window` ni de `MasterClock`** dans une init : le moteur les crée déjà.
- **Ne pas mettre dans `inits` une fonction à appeler à chaque frame**.
- **Ne pas lancer d'exception autre qu'`EngineException`** depuis une init ou un système : le `main` ne l'attrape pas et le programme s'arrête brutalement.
- **Ne pas lire `RType::Game` depuis le constructeur d'une autre variable globale** : l'ordre d'initialisation des variables globales entre fichiers n'est pas défini.

---

## 8. Les erreurs et comment les régler

### `undefined reference to 'RType::Game'`

```
Main.cpp:22: undefined reference to `RType::Game'
```

**Cause :** le jeu ne définit pas `RType::Game`.
**Solution :** ajouter la définition dans un `.cpp` du jeu (voir [section 2](#2-gameinfo--ce-que-le-jeu-fournit)), et vérifier que ce fichier est bien listé dans `add_executable`.

### `multiple definition of 'RType::Game'`

```
multiple definition of `RType::Game'; first defined here
```

**Cause :** `RType::Game` est définie dans deux `.cpp`, ou dans un header inclus par plusieurs `.cpp`.
**Solution :** garder une seule définition, dans un seul `.cpp`. Le header `Game.hpp` du moteur se charge de la déclaration.

### `designator order for field ... does not match declaration order`

```
error: designator order for field 'RType::GameInfo::windowFPS' does not match declaration order in 'RType::GameInfo'
```

**Cause :** les champs de `GameInfo` ne sont pas écrits dans l'ordre de la structure.
**Solution :** les remettre dans l'ordre `inits`, `runningCondition`, `windowFPS`, `windowWidth`, `windowHeight`, `windowName`.

### `undefined reference to 'main'`

**Cause :** le jeu n'est pas lié à `engine-game`.
**Solution :** `target_link_libraries(mon_jeu PRIVATE engine-game)`.

### `Window.hpp: No such file or directory` (ou `KeyboardManager.hpp`, `Clock.hpp`…)

**Cause :** le jeu n'est lié qu'à `engine-system`, ou à aucune bibliothèque du moteur.
**Solution :** lier `engine-game`, qui apporte les includes de `engine-system` et `engine-graphical`.

### `terminate called after throwing an instance of 'std::bad_function_call'`

```
terminate called after throwing an instance of 'std::bad_function_call'
  what():  bad_function_call
```

**Cause :** `runningCondition` n'est pas fournie : la boucle appelle une `std::function` vide.
**Solution :** définir `.runningCondition`, par exemple `[] { return !WindowShouldClose(); }`.

### `terminate called after throwing an instance of '...'`

**Cause :** une init ou un système a lancé une exception qui n'est pas une `RType::EngineException` (`std::runtime_error`, `std::out_of_range`…). Le `main` n'attrape que les `EngineException`.
**Solution :** attraper l'exception dans le jeu, ou lancer une `RType::EngineException` à la place.

### `Engine Error: Window can be created only once` (code de sortie `84`)

**Cause :** une init crée une `Window`, alors que le `main` en a déjà créé une.
**Solution :** supprimer cette création. La taille et le titre se règlent avec `windowWidth`, `windowHeight` et `windowName`.

### Le jeu ignore `RType::Game`

**Symptôme :** le jeu compile et se lance, mais aucune fenêtre ne s'ouvre et les inits ne sont pas appelées.
**Cause :** le jeu contient un `main`. Le linker l'utilise à la place de celui d'`engine-game`, sans erreur.
**Solution :** supprimer le `main` du jeu.

### Rien ne bouge

**Causes possibles :**

- **Le système de l'horloge n'est pas enregistré** : `getElapsed()` reste à zéro, donc toutes les vitesses donnent un déplacement nul. Enregistrer `MasterClock::systemMasterClock`.
- **Le système de mouvement n'est pas enregistré** : enregistrer `Velocity2D::systemMasterVelocity2D`.
- **La logique est dans `inits` au lieu d'un système** : elle n'est appelée qu'une fois. L'enregistrer avec `gm.createSystem<>(...)`.
- **Une vue demande plusieurs composants du moteur à la fois**, par exemple `view<KeyboardManager, MasterClock>()` : ils sont sur des entités différentes, la vue est vide et le système ne fait rien. Utiliser une vue par composant ([section 5](#lire-la-fenêtre-le-clavier-et-lhorloge)).

### Le clavier ne répond pas

**Causes possibles :**

- `KeyboardManager::systemKeyboardManager` n'est pas enregistré.
- `Window::systemClearWindow` n'est pas enregistré : raylib ne lit les événements du clavier et de la fenêtre qu'à la fin de chaque affichage.
- Le système qui lit le clavier est enregistré **avant** `systemKeyboardManager` : il lit les touches de la frame précédente.

### La fenêtre reste noire ou figée

**Cause :** `Window::systemClearWindow` n'est pas enregistré, ou les systèmes de dessin sont enregistrés sans lui.
**Solution :** enregistrer `Window::systemClearWindow`, puis les systèmes de dessin après lui.
