# Écrire un jeu sans `main` : `engine-game`

> 🌐 **Langue :** Français · [English](ENGINE_GAME.en.md)

`engine-game` est la bibliothèque qui fournit le `main` à la place du jeu. Le jeu donne deux choses :

- **une variable C++, `RType::GameConfig`** : la fenêtre, le nombre d'images par seconde, les modules du moteur à charger et le moment où s'arrêter ;
- **des scripts Lua** : ils créent les entités, leur donnent des composants et enregistrent les systèmes.

Le moteur s'occupe du reste : il crée le `GameManager`, charge les modules, lance chaque script, puis fait tourner la boucle.

- **Public visé :** toute personne qui crée un jeu avec le moteur.
- **Pour écrire les scripts**, consultez [le guide du scripting Lua](LUA_GUIDE.fr.md).
- **Pour l'ECS en détail** (entités, composants, systèmes, vues), consultez [le guide d'utilisation de l'ECS](ECS_GUIDE.fr.md).

> [!TIP]
> Tous les exemples de ce guide ont été compilés et lancés avec le moteur actuel. Les messages d'erreur de la [section 8](#8-les-erreurs-et-comment-les-régler) ont été reproduits avec `g++-14`.

---

## Sommaire

1. [Ce que fait le `main` du moteur](#1-ce-que-fait-le-main-du-moteur)
2. [`GameInfo` : ce que le jeu fournit](#2-gameinfo--ce-que-le-jeu-fournit)
3. [Les modules](#3-les-modules)
4. [Mettre en place le jeu](#4-mettre-en-place-le-jeu)
5. [Un jeu complet](#5-un-jeu-complet)
6. [Le mode debug](#6-le-mode-debug)
7. [À faire / à ne pas faire](#7-à-faire--à-ne-pas-faire)
8. [Les erreurs et comment les régler](#8-les-erreurs-et-comment-les-régler)

---

## 1. Ce que fait le `main` du moteur

Le `main` se trouve dans [`Engine/Game/src/Main.cpp`](../Engine/Game/src/Main.cpp) et appelle `RType::Game::Runner::run()`. Dans l'ordre, il :

1. lit le fichier `config.conf`, s'il existe, et met à jour `GameConfig` avec ses valeurs (voir [le fichier `config.conf`](#le-fichier-configconf)) ;
2. crée le `GameManager` (en mode debug si le moteur est compilé avec `__DEBUG__`, voir la [section 6](#6-le-mode-debug)) ;
3. charge les modules demandés dans `GameConfig.modules`, plus `Clock`, toujours chargé (voir la [section 3](#3-les-modules)) ;
4. charge tous les fichiers `.lua` du dossier `TestLua/` et de ses sous-dossiers, **dans l'ordre alphabétique de leur chemin**, et appelle la fonction `start(gm)` de chacun (voir [le guide Lua](LUA_GUIDE.fr.md)) ;
5. lance `gm.run(GameConfig.windowFPS, GameConfig.runningCondition)` : la boucle continue tant que `runningCondition()` renvoie `true` ;
6. si une `RType::EngineException` remonte, du moteur ou d'un script, affiche son message et quitte avec le code `84`.

Le fichier `config.conf` et le dossier `TestLua/` sont cherchés **dans le dossier depuis lequel le jeu est lancé**.

---

## 2. `GameInfo` : ce que le jeu fournit

L'en-tête [`Game.hpp`](../Engine/Game/include/Game.hpp) déclare la structure et la variable :

```cpp
namespace RType {

    struct GameInfo {
        std::function<bool()> runningCondition;
        int windowFPS = 60;
        std::size_t windowWidth = 800;
        std::size_t windowHeight = 600;
        std::string windowName = "My Game";
        std::list<std::string> modules;
    };

    extern GameInfo GameConfig;

}
```

| Champ | Rôle | Valeur par défaut |
|---|---|---|
| `runningCondition` | Appelée avant chaque frame : la boucle continue tant qu'elle renvoie `true` | **aucune, à fournir** |
| `windowFPS` | Images par seconde de la boucle | `60` |
| `windowWidth`, `windowHeight` | Taille de la fenêtre, en pixels, utilisée par le module `Window` | `800` × `600` |
| `windowName` | Titre de la fenêtre, utilisé par le module `Window` | `"My Game"` |
| `modules` | Les modules du moteur à charger, voir la [section 3](#3-les-modules) | vide (seulement `Clock`) |

Le jeu **définit** `RType::GameConfig` dans **un seul** de ses fichiers `.cpp`, avec les initialiseurs désignés de C++20. Les champs omis gardent leur valeur par défaut :

```cpp
#include "Game.hpp"

RType::GameInfo RType::GameConfig = {
    .runningCondition = [] { return !WindowShouldClose(); },
    .windowName = "Bounce",
    .modules = {"Window", "Keyboard"},
};
```

Les champs doivent être écrits **dans l'ordre de la structure** : `runningCondition`, `windowFPS`, `windowWidth`, `windowHeight`, `windowName`, `modules`. On peut en omettre, pas les intervertir.

### Le fichier `config.conf`

Tous les champs, sauf `runningCondition`, peuvent aussi être donnés dans un fichier `config.conf`, à côté du dossier `TestLua/`. Changer la fenêtre ou les modules ne demande alors pas de recompiler le jeu :

```
# fenêtre
windowFPS = 60
windowWidth = 1280
windowHeight = 720
windowName = Bounce

modules = Window, Keyboard, Mouse, Audio
```

- **Une ligne par réglage**, sous la forme `clé = valeur`. Les espaces autour de la clé, du `=` et de la valeur sont ignorés.
- **Tout est facultatif.** Une clé absente garde la valeur du `.cpp`, et sans fichier rien ne change. Si une clé apparaît deux fois, la dernière l'emporte.
- **Les valeurs du fichier remplacent celles du `.cpp`.** `modules` remplace toute la liste : il ne s'ajoute pas aux modules du `.cpp`.
- **Les lignes vides et celles qui commencent par `#` sont ignorées.**
- **`windowFPS`, `windowWidth` et `windowHeight`** sont des nombres entiers plus grands que 0. **`modules`** est une liste de noms séparés par des virgules.
- **Une ligne invalide arrête le jeu** avant que quoi que ce soit ne tourne, avec le fichier et la ligne (voir la [section 8](#8-les-erreurs-et-comment-les-régler)).

---

## 3. Les modules

Un module est une fonctionnalité du moteur que le jeu active par son nom. Chacun crée son entité et enregistre son système, **avant** les scripts : les systèmes des scripts passent donc toujours après eux.

| Module | Ce qu'il crée | A besoin de |
|---|---|---|
| `Window` | une entité avec une `Window` (taille et titre pris dans `GameConfig`), et le système qui l'efface et la rafraîchit à chaque frame | |
| `Keyboard` | une entité avec un `KeyboardManager`, et le système qui lit les touches ; les scripts le lisent avec `gm:keyboard()` | `Window` |
| `Mouse` | une entité avec un `MouseManager`, et le système qui lit la souris ; les scripts la lisent avec `gm:mouse()` | `Window` |
| `Audio` | une entité avec le point qui entend les sons, et le système qui joue les `Sound` | |
| `Clock` | une entité avec la `MasterClock`, et le système qui la fait avancer ; les scripts lisent son temps avec `gm:elapsed()` | toujours chargé |

- **`Clock` est toujours chargé**, même si `modules` ne le cite pas : les systèmes de mouvement et d'animation du moteur en dépendent.
- **Un module n'est chargé qu'une fois**, même s'il est cité deux fois.
- **L'ordre est respecté** : d'abord les modules du jeu, dans son ordre, puis `Clock`.
- **Une dépendance manquante arrête le jeu** avant que quoi que ce soit ne tourne, par exemple `Keyboard` sans `Window` (voir la [section 8](#8-les-erreurs-et-comment-les-régler)).

---

## 4. Mettre en place le jeu

### Le dossier

```
Games/
└── MyGame/
    ├── CMakeLists.txt
    ├── src/
    │   └── Setup.cpp        # seulement la définition de RType::GameConfig
    └── TestLua/
        ├── game.lua         # les scripts, chargés dans l'ordre alphabétique
        └── sprites/
            └── ship.png
```

Pas de `Main.cpp` : `engine-game` le fournit.

### Le `CMakeLists.txt`

```cmake
cmake_minimum_required(VERSION 3.25...4.0)
project(my_game CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(my_game src/Setup.cpp)
target_link_libraries(my_game PRIVATE engine-game)
```

**`engine-game` suffit** : il apporte l'ECS, le graphisme 2D et 3D, l'audio et l'interpréteur Lua, avec leurs dossiers d'en-têtes.

Ajoutez ensuite le dossier dans [`Games/CMakeLists.txt`](../Games/CMakeLists.txt) :

```cmake
add_subdirectory(MyGame)
```

et le nom du binaire dans le [`.gitignore`](../.gitignore).

### Compiler et lancer

```sh
cmake -S . -B build
cmake --build build --target my_game -j
cd Games/MyGame && ../my_game
```

Le binaire est placé dans `Games/`. Il doit être **lancé depuis le dossier qui contient `TestLua/`** : les scripts, et les chemins qu'ils donnent (images, polices…), sont lus par rapport à ce dossier.

---

## 5. Un jeu complet

Un vaisseau qui traverse la fenêtre et rebondit sur ses bords. `Échap` ou le bouton de fermeture quittent le jeu.

```cpp
// src/Setup.cpp
#include "Game.hpp"

RType::GameInfo RType::GameConfig = {
    .runningCondition = [] { return !WindowShouldClose(); },
    .windowName = "Bounce",
    .modules = {"Window", "Keyboard"},
};
```

```lua
-- TestLua/game.lua
local WIDTH, HEIGHT = 800, 600

local function bounce(gm, e, pos, vel)
    if pos.x < 0 or pos.x > WIDTH - 32 then vel.x = -vel.x end
    if pos.y < 0 or pos.y > HEIGHT - 32 then vel.y = -vel.y end
end

function start(gm)
    local ship = gm:createEntity()
    gm:addComponent(ship, 'Position2D', {x = 100, y = 100})
    gm:addComponent(ship, 'Velocity2D', {x = 250, y = 180})
    gm:addComponent(ship, 'Texture2D', {path = 'TestLua/sprites/ship.png'})
    gm:addComponent(ship, 'Sprite2D', {scale = 2})

    gm:createSystem({'Position2D', 'Velocity2D'}, bounce)
    gm:addSystem('MasterVelocity2D')
    gm:addSystem('DrawSprite2D')
end
```

- **`Setup.cpp`** active seulement la fenêtre et le clavier. Le module `Window` efface déjà la fenêtre à chaque frame.
- **`start`** tourne une fois : il crée le vaisseau, puis enregistre trois systèmes, qui tournent à chaque frame dans cet ordre. `bounce` est propre au jeu ; `MasterVelocity2D` (mouvement) et `DrawSprite2D` (dessin) viennent du moteur.

Tout ce qu'un script peut faire est dans [le guide du scripting Lua](LUA_GUIDE.fr.md).

---

## 6. Le mode debug

Quand le moteur est compilé avec la macro `__DEBUG__`, le `main` crée le `GameManager` en mode debug : chaque opération est écrite sur `std::clog`.

```sh
cmake -S . -B build -DCMAKE_CXX_FLAGS="-D__DEBUG__"
cmake --build build --target my_game -j
```

```
[GameManager] debug mode enabled
[GameManager] createEntity -> entity 0
[GameManager] addComponent<RType::MasterClock> on entity 0
[GameManager] createSystem with ( RType::MasterClock ) -> system 0
```

La macro doit s'appliquer à `engine-game`, qui contient le `main` : un `target_compile_definitions` sur la cible du jeu ne suffit pas.

---

## 7. À faire / à ne pas faire

### À faire

- **Définir `RType::GameConfig` dans un seul `.cpp`**, idéalement un `Setup.cpp` qui ne contient que ça.
- **Toujours fournir `runningCondition`**.
- **Ne lier que `engine-game`** dans le `CMakeLists.txt` du jeu.
- **Mettre `Window` dans `modules`** dès que le jeu dessine quelque chose ou lit le clavier ou la souris.
- **Lancer le jeu depuis le dossier qui contient `TestLua/`**.
- **Écrire les champs de `GameInfo` dans l'ordre de la structure**.

### À ne pas faire

- **Ne pas écrire de `main`** dans le jeu : il remplacerait celui du moteur sans prévenir.
- **Ne pas définir `RType::GameConfig` dans un en-tête**, ni dans deux `.cpp`.
- **Ne pas mettre `Clock` dans `modules`** : il est toujours chargé.
- **Ne pas lire `RType::GameConfig` depuis le constructeur d'une autre variable globale** : l'ordre d'initialisation des globales entre fichiers n'est pas défini.

---

## 8. Les erreurs et comment les régler

### `undefined reference to 'RType::GameConfig'`

```
Runner.cpp:(.text+0xa58): undefined reference to `RType::GameConfig'
```

**Cause :** le jeu ne définit pas `RType::GameConfig`.
**Solution :** ajouter la définition dans un des `.cpp` du jeu (voir la [section 2](#2-gameinfo--ce-que-le-jeu-fournit)), et vérifier que ce fichier est bien listé dans `add_executable`.

### `multiple definition of 'RType::GameConfig'`

**Cause :** `RType::GameConfig` est défini dans deux `.cpp`, ou dans un en-tête inclus par plusieurs `.cpp`.
**Solution :** garder une seule définition, dans un seul `.cpp`.

### `designator order for field ... does not match declaration order`

```
error: designator order for field 'RType::GameInfo::runningCondition' does not match declaration order in 'RType::GameInfo'
```

**Cause :** les champs de `GameInfo` ne sont pas écrits dans l'ordre de la structure.
**Solution :** les remettre dans l'ordre `runningCondition`, `windowFPS`, `windowWidth`, `windowHeight`, `windowName`, `modules`.

### `terminate called after throwing an instance of 'std::bad_function_call'`

```
terminate called after throwing an instance of 'std::bad_function_call'
  what():  bad_function_call
```

**Cause :** `runningCondition` n'est pas fourni : la boucle appelle une `std::function` vide.
**Solution :** définir `.runningCondition`, par exemple `[] { return !WindowShouldClose(); }`.

### `Engine Error: Config error: ...` (code de sortie `84`)

```
Engine Error: Config error: config.conf:2: unknown key 'fps'
Engine Error: Config error: config.conf:1: windowWidth expects a number greater than 0, got 'abc'
Engine Error: Config error: config.conf:4: expected 'key = value'
```

**Cause :** une ligne de `config.conf` a une clé qui n'existe pas, une valeur invalide, ou pas de `=`. Le message donne la ligne.
**Solution :** corriger la ligne, avec les clés et les valeurs du [fichier `config.conf`](#le-fichier-configconf). Les clés respectent la casse.

### `Engine Error: No such module: ...` (code de sortie `84`)

```
Engine Error: No such module: Windw
```

**Cause :** un nom de `modules` n'est pas un des modules de la [section 3](#3-les-modules), souvent à cause d'une faute de frappe.
**Solution :** corriger le nom. Les noms de modules respectent la casse.

### `Engine Error: Module ... requires module ...` (code de sortie `84`)

```
Engine Error: Module Keyboard requires module Window
```

**Cause :** un module est cité sans le module dont il a besoin.
**Solution :** ajouter le module manquant dans `modules`.

### `Engine Error: Script error: ...` (code de sortie `84`)

**Cause :** un script contient une erreur, ou appelle le moteur avec de mauvais arguments. Le message donne le fichier et la ligne.
**Solution :** voir [les erreurs du guide Lua](LUA_GUIDE.fr.md#9-les-erreurs-et-comment-les-régler).

### Rien ne se passe : aucun script ne tourne

**Cause :** le jeu n'est pas lancé depuis le dossier qui contient `TestLua/`. Le moteur ne trouve aucun script, et fait tourner une boucle vide sans erreur.
**Solution :** se placer dans ce dossier avant de lancer le jeu (voir la [section 4](#compiler-et-lancer)).

### La fenêtre ne s'ouvre pas

**Cause :** `Window` n'est pas dans `modules`.
**Solution :** ajouter `"Window"` dans `modules`.
