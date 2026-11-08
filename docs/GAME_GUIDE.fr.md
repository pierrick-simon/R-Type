# Créer un jeu avec le moteur

> 🌐 **Langue :** Français · [English](GAME_GUIDE.en.md)

Ce guide explique comment **créer un jeu** au-dessus du moteur : où placer le code, comment le compiler, comment l'organiser, et comment utiliser les composants fournis (fenêtre, images, animations, déplacements, clavier, souris, texte, son). Il se termine par la liste de **ce qu'il faut faire** et de **ce qu'il ne faut pas faire**.

- **Public visé :** toute personne qui commence un jeu (ou une partie du R-Type) avec le moteur.
- **Pour le détail de l'ECS** (entités, composants, systèmes, vues, frame), consultez [le guide d'utilisation de l'ECS](ECS_GUIDE.fr.md). Ce guide-ci suppose que vous avez lu au moins sa [section 1](ECS_GUIDE.fr.md#1-lessentiel-en-30-secondes).
- **Pour comprendre les choix d'architecture du moteur**, consultez [les notes de conception de l'ECS](ECS_DESIGN.fr.md).

Tout au long du guide, on construit un petit shoot'em up : un vaisseau qui se déplace au clavier et qui tire, des ennemis qui arrivent de la droite, des collisions et un score.

> [!TIP]
> Tous les exemples de ce guide ont été compilés avec le moteur actuel, et le jeu complet a été lancé avec.

---

## Sommaire

1. [Ce que fournit le moteur](#1-ce-que-fournit-le-moteur)
2. [Créer le dossier du jeu](#2-créer-le-dossier-du-jeu)
3. [Le `main` : fenêtre, entrées, boucle](#3-le-main--fenêtre-entrées-boucle)
4. [Organiser le code](#4-organiser-le-code)
5. [L'ordre des systèmes](#5-lordre-des-systèmes)
6. [Les fonctionnalités de base](#6-les-fonctionnalités-de-base)
7. [À faire](#7-à-faire)
8. [À ne pas faire](#8-à-ne-pas-faire)
9. [Limites actuelles du moteur](#9-limites-actuelles-du-moteur)
10. [Pour aller plus loin](#10-pour-aller-plus-loin)

---

## 1. Ce que fournit le moteur

Le moteur (`Engine/`) est découpé en plusieurs bibliothèques :

| Bibliothèque | Contenu | Dossier des headers |
|---|---|---|
| `engine-system` | L'ECS (`GameManager`) et les composants de base : position, horloge… | `Engine/System/include/` (et ses sous-dossiers `BuiltIn/`, `Components/`, `System/`) |
| `engine-physics` | Vitesse, frottement, gravité et rotation, en 2D et en 3D | `Engine/Physics/include/` |
| `engine-graphical2D` | Fenêtre, images, animations, texte, clavier, souris (basée sur raylib) | `Engine/Graphical2D/include/` |
| `engine-graphical3D` | Modèles 3D, sprites 3D et caméra (basée sur raylib) | `Engine/Graphical3D/include/` |
| `engine-audio` | Les sons et le point qui les entend (basée sur miniaudio) | `Engine/Audio/include/` |
| `engine-network` | Le réseau : client et serveur UDP (basé sur Asio) | `Engine/Network/include/` |

Chaque fonctionnalité est un **composant** à attacher à une entité, accompagné le plus souvent d'une **fonction statique `systemXxx`** à enregistrer comme système. Le moteur n'enregistre aucun système tout seul : c'est au jeu de le faire, et dans le bon ordre (voir [section 5](#5-lordre-des-systèmes)).

| Composant | Bibliothèque | Rôle | Système fourni (composants demandés) |
|---|---|---|---|
| `Position2D` | system | Position en pixels, `(0, 0)` en haut à gauche | — |
| `MasterClock` | system | L'horloge du jeu : temps écoulé depuis la frame précédente, une seule pour tout le jeu | `MasterClock::systemMasterClock` (`MasterClock`) |
| `Clock` | system | L'horloge propre d'une entité, pour la ralentir ou l'accélérer seule | `Clock::systemClock` (`Clock`) |
| `Origin2D` | system | Point de pivot d'un affichage (utilisé dans `Sprite2D`, `SpriteSheet2D`, `Text2D`) | — |
| `Size2D` | system | Taille entière, **dans l'ordre `{height, width}`** | — |
| `Velocity2D` | physics | Vitesse en pixels par seconde | `Velocity2D::systemMasterVelocity2D` (`MasterClock`), ou `Velocity2D::systemVelocity2D` (`Velocity2D, Position2D, Clock`) |
| `GravityField2D` | physics | Accélération appliquée à la vitesse (`x`, `y` = direction, `g` = intensité en pixels/s²) | `GravityField2D::systemMasterGravityField2D` (`MasterClock`), ou `GravityField2D::systemGravityField2D` (`GravityField2D, Velocity2D, Clock`) |
| `Friction2D` | physics | Frottement appliqué à la vitesse | `Friction2D::systemMasterFriction2D` (`MasterClock`), ou `Friction2D::systemFriction2D` (`Friction2D, Velocity2D, Clock`) |
| `Window` | graphical2D | La fenêtre | ses méthodes `beginDrawing()` et `endDrawing()`, autour des systèmes de dessin (voir [section 5](#5-lordre-des-systèmes)) |
| `Texture2D` | graphical2D | Une image chargée depuis un fichier | — |
| `Sprite2D` | graphical2D | Affiche une texture entière | `Sprite2D::systemDrawSprite2D` (`Sprite2D, Texture2D, Position2D`) |
| `SpriteSheet2D` | graphical2D | Affiche une case d'une planche d'animation, et passe à la suivante | `SpriteSheet2D::systemMasterAnimateSpriteSheet2D` (`MasterClock`) et `SpriteSheet2D::systemDrawSpriteSheet2D` (`SpriteSheet2D, Texture2D, Position2D`) |
| `Font` | graphical2D | Une police chargée depuis un fichier | — |
| `Text2D` | graphical2D | Un texte à afficher | `Text2D::systemDrawText2D` (`Text2D, Font, Position2D`) |
| `KeyboardManager` | graphical2D | L'état du clavier (touches appuyées, maintenues, relâchées) | `KeyboardManager::systemKeyboardManager` (`KeyboardManager`) |
| `MouseManager` | graphical2D | L'état de la souris (boutons, position, molette) | `MouseManager::systemMouseManager` (`MouseManager`) |
| `Keybind` | graphical2D | Réassigne des touches et des boutons | `Keybind::systemUpdateKeybind` (`Keybind, MouseManager, KeyboardManager`) |
| `Listener` | audio | Le point qui entend les sons, un seul pour tout le jeu : chaque `Sound` est créé à partir de lui | — |
| `Sound` | audio | Un son chargé depuis un fichier | `Sound::systemSound` (`Sound`) |

Tous ces types sont dans le namespace `RType`. Leur documentation complète (Doxygen) est dans leur header.

---

## 2. Créer le dossier du jeu

### La structure

Un jeu vit dans son propre dossier sous `Games/`, à côté du R-Type :

```
Games/
└── MonJeu/
    ├── CMakeLists.txt
    ├── assets/              # images, polices, sons
    ├── include/
    │   ├── Components.hpp   # les composants propres au jeu
    │   ├── Spawn.hpp        # les fonctions qui créent les entités
    │   └── Systems.hpp      # les systèmes du jeu
    └── src/
        ├── Main.cpp
        ├── Spawn.cpp
        └── Systems.cpp
```

### Le `CMakeLists.txt`

```cmake
cmake_minimum_required(VERSION 3.25...4.0)
project(mon_jeu CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRCS
        src/Main.cpp
        src/Spawn.cpp
        src/Systems.cpp
)

add_executable(mon_jeu ${SRCS})
target_include_directories(mon_jeu PRIVATE include)
target_link_libraries(mon_jeu PRIVATE engine-system engine-physics engine-graphical2D engine-audio)
target_compile_definitions(mon_jeu PRIVATE
        ASSETS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets/"
)
```

- **`engine-system`** donne l'ECS et les composants de base, **`engine-physics`** le mouvement, **`engine-graphical2D`** tout ce qui s'affiche ou se lit au clavier et à la souris, **`engine-audio`** les sons. Un serveur qui n'affiche rien n'a besoin ni d'`engine-graphical2D` ni d'`engine-audio`.
- **`ASSETS_DIR`** est le chemin absolu du dossier `assets/`. Les chemins de fichiers sont relatifs au dossier depuis lequel on lance le binaire : avec `ASSETS_DIR "player.png"`, le jeu trouve ses images d'où qu'on le lance.

Puis ajoutez le dossier dans `Games/CMakeLists.txt` :

```cmake
add_subdirectory(MonJeu)
```

### Compiler et lancer

Comme pour le reste du projet (voir le [README](../README.md)) :

```sh
cmake -S . -B build -DCMAKE_CXX_COMPILER=g++-14 -DCMAKE_C_COMPILER=gcc-14
cmake --build build --target mon_jeu -j
./Games/mon_jeu
```

Le binaire est placé directement dans `Games/`, comme `r-type_client` et `r-type_server`.

---

## 3. Le `main` : fenêtre, entrées, boucle

```cpp
#include <iostream>
#include "Clock.hpp"
#include "Exceptions.hpp"
#include "GameManager.hpp"
#include "KeyboardManager.hpp"
#include "Listener.hpp"
#include "MouseManager.hpp"
#include "Spawn.hpp"
#include "Systems.hpp"
#include "Texture2D.hpp"
#include "Window.hpp"

int main()
{
    try {
        RType::GameManager gm;

        // 1. La fenêtre d'abord : les textures et les polices en ont besoin.
        RType::Entity screen = gm.createEntity();
        gm.addComponent<RType::Window>(screen, 1280, 720, "Mon jeu");

        // 2. Les entrées, une seule fois pour tout le jeu.
        RType::Entity input = gm.createEntity();
        gm.addComponent<RType::KeyboardManager>(input);
        gm.addComponent<RType::MouseManager>(input);

        // 3. L'horloge du jeu et le point qui entend les sons.
        RType::Entity world = gm.createEntity();
        gm.addComponent<RType::MasterClock>(world);
        gm.addComponent<RType::Listener>(world);

        // 4. Tous les systèmes, avant la boucle.
        registerSystems(gm);

        // 5. Les entités de départ.
        spawnBackground(gm);
        spawnPlayer(gm, gm.getComponent<RType::Listener>(world));
        spawnScore(gm);
        spawnSpawner(gm);

        // 6. La boucle : une frame par tour, jusqu'à la fermeture de la fenêtre.
        RType::Window &window = gm.getComponent<RType::Window>(screen);
        while (!window.shouldClose())
            gm.update();

        // 7. Les textures avant la fenêtre : raylib en a besoin pour les libérer.
        RType::Texture2D::unloadTextures2D(gm);
    } catch (const RType::EngineException &e) {
        std::cerr << "Erreur : " << e.what() << std::endl;
        return 84;
    }
    return 0;
}
```

Les points importants :

- **La fenêtre est créée en premier.** `Texture2D` et `Font` en ont besoin pour se charger. Et comme `addComponent` construit le composant tout de suite, l'ordre des lignes compte.
- **Les entrées ont une entité à elles**, avec un seul `KeyboardManager` et un seul `MouseManager`. Les systèmes qui réagissent au clavier reçoivent ce `KeyboardManager` (voir [Lire le clavier et la souris](#lire-le-clavier-et-la-souris)).
- **L'horloge du jeu et le `Listener` sont créés une seule fois.** `MasterClock` donne à tous les systèmes le temps de la frame, et le `Listener` est le point qui entend les sons : chaque `Sound` est créé à partir de lui.
- **Tous les systèmes sont créés avant la boucle**, jamais pendant `update()` (voir le [piège n°2](ECS_GUIDE.fr.md#piège-n2--créer-un-système-pendant-update)).
- **`window.shouldClose()`** devient vrai quand on ferme la fenêtre (croix ou `Échap`).
- **`Texture2D::unloadTextures2D(gm)` s'exécute avant la fin du `main`.** Il détruit les entités qui portent une texture tant que la fenêtre existe encore : sans lui, le jeu plante en quittant.
- **Les erreurs du moteur héritent toutes de `RType::EngineException`** : une seule clause `catch` suffit. Le détail est dans [la gestion des erreurs](ECS_GUIDE.fr.md#6-la-gestion-des-erreurs).

---

## 4. Organiser le code

### Les composants du jeu

Les composants propres au jeu sont de simples structs de données, regroupées dans un header :

```cpp
// include/Components.hpp
#include <chrono>

struct Player {
    float speed = 400.f;
};

struct Enemy {
    int hp = 3;
};

struct Bullet {};            // tag : « cette entité est une balle »

struct DestroyOffScreen {};  // tag : « détruire cette entité quand elle sort de l'écran »

struct Hitbox {
    float width = 0;
    float height = 0;
};

struct Score {
    int value = 0;
};

struct Spawner {
    std::chrono::milliseconds every = std::chrono::milliseconds(1500);
    std::chrono::milliseconds elapsed = std::chrono::milliseconds(0);
};
```

Les **tags** (structs vides) servent à trier les entités : un système sur `Bullet` ne voit que les balles. On combine ainsi les comportements sans hiérarchie de classes. Les règles d'écriture d'un composant sont dans [Écrire un composant](ECS_GUIDE.fr.md#écrire-un-composant).

### Une fonction par type d'entité

Chaque « sorte » d'entité est créée par une fonction `spawnXxx`, qui lui attache tous ses composants :

```cpp
// src/Spawn.cpp
RType::Entity spawnPlayer(RType::GameManager &gm, RType::Listener &listener)
{
    RType::Entity player = gm.createEntity();

    gm.addComponent<Player>(player);
    gm.addComponent<RType::Position2D>(player, 100.f, 300.f);
    gm.addComponent<RType::Velocity2D>(player);
    gm.addComponent<Hitbox>(player, 64.f, 32.f);
    gm.addComponent<RType::Texture2D>(player, ASSETS_DIR "player.png");
    gm.addComponent<RType::Sprite2D>(player);
    gm.addComponent<RType::Sound>(player, listener, ASSETS_DIR "shot.mp3");
    return player;
}

RType::Entity spawnBullet(RType::GameManager &gm, RType::Position2D pos)
{
    RType::Entity bullet = gm.createEntity();

    gm.addComponent<Bullet>(bullet);
    gm.addComponent<RType::Position2D>(bullet, pos);
    gm.addComponent<RType::Velocity2D>(bullet, 800.f, 0.f);
    gm.addComponent<Hitbox>(bullet, 16.f, 8.f);
    gm.addComponent<DestroyOffScreen>(bullet);
    gm.addComponent<RType::Texture2D>(bullet, ASSETS_DIR "bullet.png");
    gm.addComponent<RType::Sprite2D>(bullet);
    return bullet;
}
```

Ces fonctions s'appellent aussi bien au démarrage que depuis un système, pendant la partie : `createEntity` est toujours immédiat, et les `addComponent` sont appliqués à la fin de la frame (voir [la frame](ECS_GUIDE.fr.md#5-la-frame--opérations-immédiates-et-différées)).

### Une fonction qui enregistre les systèmes

Les systèmes sont des fonctions libres, déclarées dans `Systems.hpp` et écrites dans `Systems.cpp` (voir [Écrire ses systèmes dans des fichiers](ECS_GUIDE.fr.md#écrire-ses-systèmes-dans-des-fichiers)). Une fonction `registerSystems` les enregistre tous, au même endroit, dans l'ordre d'exécution : c'est l'objet de la section suivante.

---

## 5. L'ordre des systèmes

Les systèmes s'exécutent **dans l'ordre où ils ont été créés**. Cet ordre décide de ce que chaque système voit : un système de déplacement placé avant la lecture du clavier utilise les touches de la frame précédente. Voici l'ordre conseillé :

```cpp
void registerSystems(RType::GameManager &gm)
{
    // 1. Début de la frame : images par seconde, puis effacer l'écran
    gm.createSystem<RType::Window>(
        [](RType::Window &window) {
            window.setTargetFps();
            window.beginDrawing();
        },
        std::numeric_limits<int>::min());

    // 2. Entrées : lire le clavier et la souris
    gm.createSystem<RType::KeyboardManager>(RType::KeyboardManager::systemKeyboardManager);
    gm.createSystem<RType::MouseManager>(RType::MouseManager::systemMouseManager);

    // 3. Temps : faire avancer l'horloge du jeu
    gm.createSystem<RType::MasterClock>(RType::MasterClock::systemMasterClock);

    // 4. Logique du jeu
    gm.createSystem<RType::KeyboardManager>(steerPlayer);
    gm.createSystem<RType::KeyboardManager>(shoot);
    gm.createSystem<RType::MasterClock>(spawnEnemies);
    gm.createSystem(collisions);
    gm.createSystem(destroyOffScreen);
    gm.createSystem<Score, RType::Text2D>(showScore);

    // 5. Physique : appliquer les vitesses
    gm.createSystem<RType::MasterClock>(RType::Velocity2D::systemMasterVelocity2D);

    // 6. Animation
    gm.createSystem<RType::MasterClock>(
        RType::SpriteSheet2D::systemMasterAnimateSpriteSheet2D);

    // 7. Affichage : le fond d'abord, puis les sprites, les planches et les textes par-dessus
    gm.createSystem<Background, RType::Texture2D, RType::Position2D>(drawBackground);
    gm.createSystem<RType::Sprite2D, RType::Texture2D, RType::Position2D>(
        RType::Sprite2D::systemDrawSprite2D);
    gm.createSystem<RType::SpriteSheet2D, RType::Texture2D, RType::Position2D>(
        RType::SpriteSheet2D::systemDrawSpriteSheet2D);
    gm.createSystem<RType::Text2D, RType::Font, RType::Position2D>(
        RType::Text2D::systemDrawText2D);

    // 8. Son
    gm.createSystem<RType::Sound>(RType::Sound::systemSound);

    // 9. Fin de la frame : afficher ce qui a été dessiné
    gm.createSystem<RType::Window>(
        [](RType::Window &window) { window.endDrawing(); },
        std::numeric_limits<int>::max());
}
```

- **Les systèmes du moteur s'enregistrent comme les vôtres** : `gm.createSystem<Composants...>(Classe::systemXxx)`, avec les composants dans le même ordre que les paramètres de la fonction. Les paramètres `const` sont acceptés.
- **Tout dessin se fait entre `beginDrawing()` et `endDrawing()`.** Le second argument de `createSystem` est la couche du système : les systèmes s'exécutent par couche, puis dans l'ordre de création. `std::numeric_limits<int>::min()` place le début de la frame avant tout le reste, et `std::numeric_limits<int>::max()` sa fin après tout le reste, où que les autres systèmes soient créés.
- **Les systèmes `Master…` prennent la `MasterClock`** : `Velocity2D::systemMasterVelocity2D` déplace toutes les entités qui ont une `Position2D` et une `Velocity2D`, et `SpriteSheet2D::systemMasterAnimateSpriteSheet2D` anime toutes les `SpriteSheet2D`. Ils tournent une fois par frame, puisqu'il n'y a qu'une `MasterClock`.
- **`KeyboardManager::systemKeyboardManager` et `MasterClock::systemMasterClock` doivent s'exécuter exactement une fois par frame**, avant tout système qui lit le clavier ou le temps.
- **L'ordre d'affichage suit l'ordre des systèmes** : le fond est dessiné, puis les sprites, les planches animées, et les textes par-dessus. À l'intérieur d'un même système, l'ordre des entités n'est pas garanti (voir [Un fond d'écran](#un-fond-décran)).

---

## 6. Les fonctionnalités de base

### Afficher une image

Une image à l'écran, c'est trois composants sur la même entité : `Texture2D` (l'image), `Sprite2D` (comment la dessiner) et `Position2D` (où).

```cpp
RType::Entity ship = gm.createEntity();
gm.addComponent<RType::Position2D>(ship, 100.f, 300.f);
gm.addComponent<RType::Texture2D>(ship, ASSETS_DIR "player.png");

RType::Sprite2D sprite;
sprite.scale = 2.f;
sprite.flip = true;      // miroir horizontal
gm.addComponent<RType::Sprite2D>(ship, sprite);
```

`Sprite2D` contient aussi `origin` (le pivot, voir `Origin2D`), `rotation` en degrés et `color` (une teinte, blanc par défaut). Pour changer ces valeurs, le plus lisible est de créer l'objet, de modifier ses champs, puis de le passer à `addComponent`, comme ci-dessus.

### Animer une planche de sprites

Une planche (*sprite sheet*) est une image découpée en une grille de cases de même taille. `SpriteSheet2D` affiche une case, puis passe à la suivante toutes les `frameDuration` :

```cpp
RType::Entity enemy = gm.createEntity();
gm.addComponent<RType::Position2D>(enemy, 1100.f, 300.f);
gm.addComponent<RType::Texture2D>(enemy, ASSETS_DIR "enemy.png");

RType::SpriteSheet2D sheet;
sheet.recNb = {1, 4};                               // 1 ligne, 4 colonnes
sheet.frameDuration = std::chrono::milliseconds(80);
gm.addComponent<RType::SpriteSheet2D>(enemy, sheet);
```

- **`recNb` est un `Size2D`, dont l'ordre est `{height, width}`** : `{1, 4}` veut dire 1 ligne et 4 colonnes. Aucune des deux valeurs ne doit valoir 0.
- **`SpriteSheet2D::systemMasterAnimateSpriteSheet2D` l'anime** avec l'horloge du jeu. `animation = false` fige la case courante, et `currentTile` choisit la case affichée.

### Déplacer une entité

`Velocity2D` déplace la `Position2D` de son entité, en pixels par seconde :

```cpp
gm.addComponent<RType::Position2D>(bullet, 100.f, 300.f);
gm.addComponent<RType::Velocity2D>(bullet, 800.f, 0.f);   // 800 px/s vers la droite
```

- **`Velocity2D::systemMasterVelocity2D` la déplace** avec l'horloge du jeu ([section 5](#5-lordre-des-systèmes)).
- **Le repère est celui de raylib** : `(0, 0)` en haut à gauche, `x` vers la droite, `y` vers le bas.
- **Pour une gravité**, ajoutez `GravityField2D` et son système (avant celui de `Velocity2D`). Ses champs sont `x`, `y` (la direction, vers le bas par défaut) et `g`, l'intensité **en pixels/s²**. La valeur par défaut (`9.80665`) est donc presque invisible à l'écran : comptez plutôt quelques centaines.

```cpp
gm.addComponent<RType::GravityField2D>(player, 0.f, 1.f, 900.f);   // vers le bas, 900 px/s²

gm.createSystem<RType::MasterClock>(
    RType::GravityField2D::systemMasterGravityField2D);
```

- **Pour ralentir ou accélérer certaines entités seulement**, donnez-leur leur propre `Clock` et utilisez les systèmes qui le prennent, à la place des `Master…`. `Clock::speed` multiplie le temps vu par l'entité : `0` la fige (mouvement et animation), `2` la fait aller deux fois plus vite. C'est un entier : pas de ralenti.

```cpp
gm.addComponent<RType::Clock>(bullet);

gm.createSystem<RType::Clock>(RType::Clock::systemClock);
gm.createSystem<RType::Velocity2D, RType::Position2D, RType::Clock>(
    RType::Velocity2D::systemVelocity2D);
```

### Lire le clavier et la souris

Le `KeyboardManager` et le `MouseManager` de l'entité des entrées (voir [section 3](#3-le-main--fenêtre-entrées-boucle)) sont remis à jour à chaque frame par leur système. Ils exposent trois listes :

| Liste | Contenu |
|---|---|
| `keyPressed` / `buttonPressed` | Les touches et boutons enfoncés **pendant cette frame** (un seul tour) |
| `keyDown` / `buttonDown` | Les touches et boutons **maintenus** |
| `keyReleased` / `buttonReleased` | Les touches et boutons relâchés **pendant cette frame** |

Le `MouseManager` donne aussi `getPos()` (la position du curseur, en `Position2D`), `getDelta()` (son déplacement depuis la frame précédente) et `getWheel()` (la molette).

Un système les lit en demandant le manager : `gm.createSystem<RType::KeyboardManager>(steerPlayer)` appelle `steerPlayer(gm, keyboard)` une fois par frame, puisqu'il n'y a qu'un `KeyboardManager`. Le système parcourt ensuite les entités qu'il fait bouger avec une vue ([Contrôler le joueur](#contrôler-le-joueur)).

Les touches sont des `RType::Key::Value` (`Key::Value::Space`, `Key::Value::Left`…). La disposition par défaut est AZERTY ; pour un clavier QWERTY : `gm.addComponent<RType::KeyboardManager>(input, RType::Key::Layout::QWERTY)`.

**Réassigner des touches** se fait avec un `Keybind`, sur l'entité des entrées, et son système placé **juste après** ceux du clavier et de la souris :

```cpp
gm.addComponent<RType::Keybind>(input);
RType::Keybind &bind = gm.getComponent<RType::Keybind>(input);
bind._keyToKey[RType::Key::Value::Z] = RType::Key::Value::Up;                       // Z agit comme ↑
bind._mouseButtonToKey[RType::MouseButton::Value::Left] = RType::Key::Value::Space; // clic = tir

gm.createSystem<RType::Keybind, RType::MouseManager, RType::KeyboardManager>(
    RType::Keybind::systemUpdateKeybind);
```

### Contrôler le joueur

Un système sur le `KeyboardManager` transforme les touches maintenues en vitesse pour chaque `Player`. Le déplacement lui-même est fait par le système de `Velocity2D`.

```cpp
static bool contains(const std::vector<RType::Key::Value> &keys, RType::Key::Value key)
{
    return std::ranges::find(keys, key) != keys.end();
}

void steerPlayer(RType::GameManager &gm, const RType::KeyboardManager &keyboard)
{
    const auto &down = keyboard.keyDown;

    for (auto &[e, player, vel] : gm.view<Player, RType::Velocity2D>().getVector()) {
        vel = {0.f, 0.f};
        if (contains(down, RType::Key::Value::Left))
            vel.x -= player.speed;
        if (contains(down, RType::Key::Value::Right))
            vel.x += player.speed;
        if (contains(down, RType::Key::Value::Up))
            vel.y -= player.speed;
        if (contains(down, RType::Key::Value::Down))
            vel.y += player.speed;
    }
}

gm.createSystem<RType::KeyboardManager>(steerPlayer);
```

### Tirer : créer des entités pendant la partie

De la même façon, un système sur le `KeyboardManager` crée une balle et joue le son du joueur quand `Espace` vient d'être pressée. Il parcourt les joueurs avec `getVector()` (voir [Parcourir une vue dans un système](ECS_GUIDE.fr.md#parcourir-une-vue-dans-un-système--getvector)) :

```cpp
void shoot(RType::GameManager &gm, const RType::KeyboardManager &keyboard)
{
    if (!contains(keyboard.keyPressed, RType::Key::Value::Space))
        return;
    for (auto &[e, player, pos, sound] :
        gm.view<Player, RType::Position2D, RType::Sound>().getVector()) {
        spawnBullet(gm, {pos.x + 64.f, pos.y + 12.f});
        sound.state = RType::Sound::State::PLAY;
    }
}

gm.createSystem<RType::KeyboardManager>(shoot);
```

On utilise `keyPressed` et non `keyDown` : sinon, une balle partirait à chaque frame tant que la touche est maintenue.

### Faire apparaître des ennemis à intervalle régulier

Une entité « générateur » porte un `Spawner`. Un système sur la `MasterClock` lui ajoute le temps de la frame, et crée un ennemi quand le délai est atteint :

```cpp
void spawnEnemies(RType::GameManager &gm, const RType::MasterClock &clock)
{
    static std::mt19937 rng(std::random_device {}());
    std::uniform_real_distribution<float> y(0.f, 720.f - 48.f);

    for (auto &[e, spawner] : gm.view<Spawner>().getVector()) {
        spawner.elapsed += clock.getElapsed();
        if (spawner.elapsed < spawner.every)
            continue;
        spawner.elapsed = std::chrono::milliseconds(0);
        spawnEnemy(gm, {1280.f, y(rng)});
    }
}

// dans le main
RType::Entity spawner = gm.createEntity();
gm.addComponent<Spawner>(spawner);

// dans registerSystems
gm.createSystem<RType::MasterClock>(spawnEnemies);
```

`spawnEnemy` suit le modèle de `spawnBullet`, avec `Enemy`, une `Velocity2D` vers la gauche (`-150.f, 0.f`) et une `SpriteSheet2D`.

### Les collisions

Le moteur ne fournit pas encore de collisions : on les écrit avec un composant `Hitbox` et deux vues imbriquées. La liste des ennemis est construite **une seule fois**, avant la boucle sur les balles :

```cpp
static bool overlap(const RType::Position2D &a, const Hitbox &ha,
    const RType::Position2D &b, const Hitbox &hb)
{
    return a.x < b.x + hb.width && b.x < a.x + ha.width &&
        a.y < b.y + hb.height && b.y < a.y + ha.height;
}

void collisions(RType::GameManager &gm)
{
    auto enemies = gm.view<Enemy, RType::Position2D, Hitbox>().getVector();

    for (auto &[bullet, b, bPos, bBox] :
        gm.view<Bullet, RType::Position2D, Hitbox>().getVector()) {
        for (auto &[enemy, en, ePos, eBox] : enemies) {
            if (en.hp <= 0 || !overlap(bPos, bBox, ePos, eBox))
                continue;
            gm.destroyEntity(bullet);
            if (--en.hp <= 0) {
                gm.destroyEntity(enemy);
                for (auto &[e, score] : gm.view<Score>().getVector())
                    score.value += 100;
            }
            break;   // une balle ne touche qu'un ennemi
        }
    }
}
```

Le test `en.hp <= 0` est important : `destroyEntity` est différé, donc un ennemi tué reste dans la liste jusqu'à la fin de la frame. Sans ce test, une deuxième balle pourrait le « tuer » une seconde fois et compter les points en double.

### Détruire ce qui sort de l'écran

Le tag `DestroyOffScreen` désigne les entités à supprimer quand elles quittent l'écran (balles et ennemis, pas le joueur) :

```cpp
void destroyOffScreen(RType::GameManager &gm)
{
    for (auto &[e, tag, pos] :
        gm.view<DestroyOffScreen, RType::Position2D>().getVector()) {
        if (pos.x < -100.f || pos.x > 1280.f + 100.f)
            gm.destroyEntity(e);
    }
}
```

Sans ce système, les balles continueraient d'exister, de bouger et d'être dessinées hors de l'écran, indéfiniment. Pour une durée de vie fixe (explosions, particules), voir la recette [Une durée de vie limitée](ECS_GUIDE.fr.md#une-durée-de-vie-limitée-balles-particules-explosions).

### Afficher du texte

Un texte, c'est `Text2D` (le contenu et le style), `Font` (la police) et `Position2D` :

```cpp
RType::Entity score = gm.createEntity();
gm.addComponent<Score>(score);
gm.addComponent<RType::Position2D>(score, 20.f, 20.f);
gm.addComponent<RType::Font>(score, ASSETS_DIR "font.ttf");

RType::Text2D text;
text.fontSize = 32.f;
text.str = "Score: 0";
gm.addComponent<RType::Text2D>(score, text);
```

`Text2D` contient aussi `color`, `rotation`, `spacing` et `origin`. Pour un texte qui change, un système met à jour `str` à partir des données du jeu :

```cpp
void showScore(const Score &score, RType::Text2D &text)
{
    text.str = "Score: " + std::to_string(score.value);
}

gm.createSystem<Score, RType::Text2D>(showScore);
```

### Jouer un son

Un `Sound` se crée à partir du `Listener` du jeu (voir [section 3](#3-le-main--fenêtre-entrées-boucle)), puis se pilote par son champ `state`, que `Sound::systemSound` applique à son prochain passage :

```cpp
gm.addComponent<RType::Sound>(player, listener, ASSETS_DIR "shot.mp3");   // loop = false par défaut

// dans un système
sound.state = RType::Sound::State::PLAY;     // jouer depuis le début
sound.state = RType::Sound::State::PAUSED;   // mettre en pause
sound.state = RType::Sound::State::START;    // reprendre après une pause
sound.state = RType::Sound::State::STOP;     // arrêter
```

Pour une musique de fond, passez un `SoundInfo` avec `loop = true` en troisième argument, puis mettez `state` à `PLAY` juste après l'ajout :

```cpp
gm.addComponent<RType::Sound>(music, listener, ASSETS_DIR "music.mp3", RType::SoundInfo {.loop = true});
gm.getComponent<RType::Sound>(music).state = RType::Sound::State::PLAY;
```

`sound.setVolume(v)` et `sound.setPitch(p)` changent le volume et la hauteur (`1` par défaut).

### Un fond d'écran

À l'intérieur d'un même système, l'ordre de dessin des entités n'est pas garanti : un fond affiché avec `Sprite2D` peut passer devant le vaisseau. La solution est de lui donner **son propre système de dessin**, enregistré avant les autres, et **pas** de composant `Sprite2D` :

```cpp
struct Background {};

void drawBackground(const Background &, const RType::Texture2D &texture,
    const RType::Position2D &pos)
{
    RType::Sprite2D::systemDrawSprite2D(RType::Sprite2D {}, texture, pos);
}

// dans registerSystems, avant les autres systèmes de dessin
gm.createSystem<Background, RType::Texture2D, RType::Position2D>(drawBackground);
```

---

## 7. À faire

- **Créer la fenêtre en premier**, avant tout `Texture2D` ou `Font`.
- **Créer une seule `MasterClock` et un seul `Listener`** au démarrage, et créer les sons à partir de ce `Listener`.
- **Créer tous les systèmes au démarrage**, dans une seule fonction (`registerSystems`), dans l'ordre début de frame → entrées → temps → logique → physique → animation → affichage → son → fin de frame ([section 5](#5-lordre-des-systèmes)).
- **Mettre le début et la fin de la frame sur la couche la plus basse et la plus haute** (`std::numeric_limits<int>::min()` et `max()`), pour que tout dessin se fasse entre les deux.
- **Utiliser les systèmes `Master…`** pour déplacer et animer avec l'horloge du jeu, et un `Clock` seulement sur les entités qui doivent avoir leur propre temps.
- **Écrire de petits composants**, chacun avec un seul rôle, et **utiliser des tags** (structs vides) pour trier les entités. `Bullet` + `DestroyOffScreen` vaut mieux qu'un `Bullet` qui sait tout faire.
- **Créer chaque sorte d'entité dans une fonction `spawnXxx`** : le même code sert au démarrage et en cours de partie.
- **Garder l'état du jeu dans des composants** (score, vies, niveau) : les systèmes le retrouvent avec une vue, et il disparaît avec son entité.
- **Parcourir les vues avec `getVector()` dans les systèmes par frame**, et construire une seule fois une liste utilisée dans une boucle imbriquée ([collisions](#les-collisions)).
- **Garder les `Entity` dont on a besoin plus tard**, et vérifier `gm.hasEntity(e)` avant de s'en servir : une entité peut avoir été détruite entre-temps ([piège n°4](ECS_GUIDE.fr.md#piège-n4--conserver-un-pointeur-ou-une-référence-vers-le-composant-dune-autre-entité), [recette du projectile](ECS_GUIDE.fr.md#un-projectile-qui-vise-une-cible-susceptible-dêtre-détruite)).
- **Appeler `Texture2D::unloadTextures2D(gm)` à la fin du `main`**, avant que la fenêtre ne disparaisse.
- **Attraper `RType::EngineException` dans le `main`** et renvoyer `84`.
- **Passer les chemins des fichiers par `ASSETS_DIR`** ([section 2](#le-cmakeliststxt)).
- **Tester la logique sans fenêtre** : les systèmes qui n'utilisent que des composants du jeu et `engine-system` (collisions, score, destruction) se testent avec un simple `GameManager`, sans raylib, par exemple dans les tests Criterion.
- **Lancer `./script/clang.sh` et `./script/lint.sh` avant de pousser.**

---

## 8. À ne pas faire

- **Ne pas utiliser `EntityManager`, `ComponentManager` ou `SystemManager`** : tout passe par le `GameManager`.
- **Ne pas charger une texture ou une police avant la fenêtre** : le chargement échoue ou plante.
- **Ne pas créer deux `Window` ou deux `Listener`** : le second constructeur lève une `EngineException`. De même, **un seul `KeyboardManager`, un seul `MouseManager` et une seule `MasterClock`** pour tout le jeu.
- **Ne pas détruire le `Listener` avant les sons** : chaque `Sound` s'en sert jusqu'à sa destruction.
- **Ne pas dessiner en dehors de `beginDrawing()` et `endDrawing()`** : gardez le début et la fin de la frame sur les couches extrêmes ([section 5](#5-lordre-des-systèmes)).
- **Ne pas créer de système pendant `update()`** ([piège n°2](ECS_GUIDE.fr.md#piège-n2--créer-un-système-pendant-update)) : tous les systèmes au démarrage. Pour désactiver un comportement, retirez un composant ou utilisez un tag ([recette](ECS_GUIDE.fr.md#mettre-une-entité-en-pause-sans-détruire-de-système)).
- **Ne pas appeler `gm.update()` depuis un système** : l'appel ne fait rien.
- **Ne pas lire un composant juste après l'avoir ajouté dans un système** : il n'existe qu'à la fin de la frame ([piège n°7](ECS_GUIDE.fr.md#piège-n7--lire-un-composant-juste-après-lavoir-ajouté-dans-un-système)).
- **Ne pas mettre de logique ni d'appel au `GameManager` dans un composant** : la logique va dans les systèmes ([piège n°1](ECS_GUIDE.fr.md#piège-n1--appeler-le-gamemanager-depuis-le-constructeur-ou-le-destructeur-dun-composant)).
- **Ne pas inverser `Size2D`** : c'est `{height, width}`, pas `{width, height}`.
- **Ne pas enregistrer un système `Master…` et le système à `Clock` correspondant pour les mêmes entités** : elles bougeraient deux fois par frame.
- **Ne pas compter sur l'ordre des entités dans un système** : pour un ordre d'affichage, utilisez des systèmes séparés ([fond d'écran](#un-fond-décran)).
- **Ne pas laisser vivre les entités inutiles** : balles hors de l'écran, explosions terminées… Chaque entité oubliée continue d'être mise à jour et dessinée.
- **Ne pas modifier le moteur pour un besoin propre au jeu** : écrivez un composant et un système côté jeu. Si le moteur doit évoluer, voyez-le avec l'équipe moteur.
- **Ne pas lancer `cmake .` à la racine** : utilisez toujours `-B build` (voir le [README](../README.md)).

---

## 9. Limites actuelles du moteur

Ce que le moteur ne fait pas encore, et comment faire en attendant :

| Limite | En attendant |
|---|---|
| Pas d'ordre d'affichage (profondeur) à l'intérieur d'un système | Un système de dessin par « couche » ([fond d'écran](#un-fond-décran)) |
| Pas de collisions | Un composant `Hitbox` et un système côté jeu ([collisions](#les-collisions)) |
| `Clock::speed` est entier | Pas de ralenti ; `0` pour figer, `1` pour la vitesse normale |

---

## 10. Pour aller plus loin

| Document | Contenu |
|---|---|
| [README](../README.md) | Installation, compilation, tests, linter |
| [Guide d'utilisation de l'ECS](ECS_GUIDE.fr.md) | Toute l'API du `GameManager`, en détail |
| Guide de l'ECS, [Les systèmes](ECS_GUIDE.fr.md#4-les-systèmes) | Systèmes par entité, par frame, avec le `GameManager`, `getVector()` |
| Guide de l'ECS, [La frame](ECS_GUIDE.fr.md#5-la-frame--opérations-immédiates-et-différées) | Ce qui est immédiat et ce qui est différé pendant `update()` |
| Guide de l'ECS, [Les pièges à éviter](ECS_GUIDE.fr.md#7-les-pièges-à-éviter) | Les erreurs les plus courantes, avec leur explication |
| Guide de l'ECS, [Recettes](ECS_GUIDE.fr.md#8-recettes) | Durée de vie, pause, projectile à tête chercheuse, collisions |
| Guide de l'ECS, [Aide-mémoire de l'API](ECS_GUIDE.fr.md#9-aide-mémoire-de-lapi) | Toutes les méthodes sur une page |
| [Notes de conception de l'ECS](ECS_DESIGN.fr.md) | Le pourquoi de l'architecture du moteur |
| [Accessibilité](ACCESSIBILITY.fr.md) | Ce que le moteur offre pour l'accessibilité, pourquoi c'est utile et pour qui |
| [Études comparatives](comparative-studies/fr/README.md) | Les choix techniques du projet et leurs alternatives : langage, build, ECS, raylib, réseau, protocole, tests… |
| `Engine/*/include/` | La documentation Doxygen de chaque composant fourni |
| `tests/engine/` | Les tests unitaires de l'ECS : un exemple d'utilisation pour chaque comportement |
