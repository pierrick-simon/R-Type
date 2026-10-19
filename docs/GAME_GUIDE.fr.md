# Créer un jeu avec le moteur

> 🌐 **Langue :** Français · [English](GAME_GUIDE.en.md)

Ce guide explique comment **créer un jeu** au-dessus du moteur : où placer le code, comment le compiler, comment l'organiser, et comment utiliser les composants fournis (fenêtre, images, animations, déplacements, clavier, souris, texte, son). Il se termine par la liste de **ce qu'il faut faire** et de **ce qu'il ne faut pas faire**.

- **Public visé :** toute personne qui commence un jeu (ou une partie du R-Type) avec le moteur.
- **Pour le détail de l'ECS** (entités, composants, systèmes, vues, frame), consultez [le guide d'utilisation de l'ECS](ECS_GUIDE.fr.md). Ce guide-ci suppose que vous avez lu au moins sa [section 1](ECS_GUIDE.fr.md#1-lessentiel-en-30-secondes).
- **Pour comprendre les choix d'architecture du moteur**, consultez [les notes de conception de l'ECS](ECS_DESIGN.fr.md).

Tout au long du guide, on construit un petit shoot'em up : un vaisseau qui se déplace au clavier et qui tire, des ennemis qui arrivent de la droite, des collisions et un score.

> [!TIP]
> Tous les exemples de ce guide ont été compilés avec le moteur actuel. La logique du jeu (collisions, destruction hors de l'écran, score) a aussi été exécutée sans fenêtre.

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

Le moteur (`Engine/`) est découpé en trois bibliothèques :

| Bibliothèque | Contenu | Dossier des headers |
|---|---|---|
| `engine-system` | L'ECS (`GameManager`) et les composants de base : position, vitesse, gravité, horloge… | `Engine/System/include/` (et ses sous-dossiers `BuiltIn/`, `Components/`, `System/`) |
| `engine-graphical` | Fenêtre, images, animations, texte, son, clavier, souris (basée sur raylib) | `Engine/Graphical/include/` |
| `engine-network` | Le réseau : **pas encore implémenté** | `Engine/Network/include/` |

Chaque fonctionnalité est un **composant** à attacher à une entité, accompagné le plus souvent d'une **fonction statique `systemXxx`** à enregistrer comme système. Le moteur n'enregistre aucun système tout seul : c'est au jeu de le faire, et dans le bon ordre (voir [section 5](#5-lordre-des-systèmes)).

| Composant | Bibliothèque | Rôle | Système fourni (composants demandés) |
|---|---|---|---|
| `Position2D` | system | Position en pixels, `(0, 0)` en haut à gauche | — |
| `Velocity2D` | system | Vitesse en pixels par seconde | `Velocity2D::systemVelocity2D` (`Velocity2D, Position2D, Clock`) |
| `GravityField2D` | system | Accélération appliquée à la vitesse (`x`, `y` = direction, `g` = intensité en pixels/s²) | `GravityField2D::systemGravityField2D` (`GravityField2D, Velocity2D, Clock`) |
| `Friction2D` | system | Frottement appliqué à la vitesse | `Friction2D::systemFriction2D` (`Friction2D, Velocity2D, Clock`) |
| `Clock` | system | Temps écoulé depuis la frame précédente | `Clock::systemClock` (`Clock`) |
| `Origin2D` | system | Point de pivot d'un affichage (utilisé dans `Sprite2D`, `SpriteSheet2D`, `Text2D`) | — |
| `Size2D` | system | Taille entière, **dans l'ordre `{height, width}`** | — |
| `Window` | graphical | La fenêtre, et le périphérique audio | `Window::systemClearWindow` (`Window`) |
| `Texture2D` | graphical | Une image chargée depuis un fichier | — |
| `Sprite2D` | graphical | Affiche une texture entière | `Sprite2D::systemDrawSprite2D` (`Sprite2D, Texture2D, Position2D`) |
| `SpriteSheet2D` | graphical | Affiche une case d'une planche d'animation, et passe à la suivante | `SpriteSheet2D::systemAnimateSpriteSheet2D` (`SpriteSheet2D, Clock`) et `SpriteSheet2D::systemDrawSpriteSheet2D` (`SpriteSheet2D, Texture2D, Position2D`) |
| `Font` | graphical | Une police chargée depuis un fichier | — |
| `Text2D` | graphical | Un texte à afficher | `Text2D::systemDrawText2D` (`Text2D, Font, Position2D`) |
| `Sound` | graphical | Un son chargé depuis un fichier | `Sound::systemSound` (`Sound`) |
| `KeyboardManager` | graphical | L'état du clavier (touches appuyées, maintenues, relâchées) | `KeyboardManager::systemKeyboardManager` (`KeyboardManager`) |
| `MouseManager` | graphical | L'état de la souris (boutons, position, molette) | `MouseManager::systemMouseManager` (`MouseManager`) |
| `Keyboard`, `Mouse` | graphical | Un lien vers le `KeyboardManager` ou le `MouseManager`, à mettre sur les entités qui réagissent aux entrées | — |
| `Keybind` | graphical | Réassigne des touches et des boutons | `Keybind::systemUpdateKeybind` (`Keybind, MouseManager, KeyboardManager`) |

Tous ces types sont dans le namespace `RType`. Leur documentation complète (Doxygen) est dans leur header.

---

## 2. Créer le dossier du jeu

### La structure

Un jeu vit dans son propre dossier sous `Games/`, à côté du R-Type et du Poc :

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
target_link_libraries(mon_jeu PRIVATE engine-system engine-graphical)
target_compile_definitions(mon_jeu PRIVATE
        ASSETS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets/"
)
```

- **`engine-system`** donne l'ECS et les composants de base, **`engine-graphical`** tout ce qui s'affiche, s'entend ou se lit au clavier. Un serveur qui n'affiche rien n'a besoin que d'`engine-system`.
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
#include "Components.hpp"
#include "Exceptions.hpp"
#include "GameManager.hpp"
#include "KeyboardManager.hpp"
#include "MouseManager.hpp"
#include "Spawn.hpp"
#include "Systems.hpp"
#include "Window.hpp"

int main()
{
    try {
        RType::GameManager gm;

        // 1. La fenêtre d'abord : textures, polices et sons en ont besoin.
        RType::Entity screen = gm.createEntity();
        gm.addComponent<RType::Window>(screen, 1280, 720, "Mon jeu");

        // 2. Les entrées, une seule fois pour tout le jeu.
        RType::Entity input = gm.createEntity();
        gm.addComponent<RType::KeyboardManager>(input);
        gm.addComponent<RType::MouseManager>(input);

        // 3. Tous les systèmes, avant la boucle.
        registerSystems(gm);

        // 4. Les entités de départ.
        spawnPlayer(gm, gm.getComponent<RType::KeyboardManager>(input));
        spawnScore(gm);

        // 5. La boucle : une frame par tour, jusqu'à la fermeture de la fenêtre.
        while (!WindowShouldClose())
            gm.update();
    } catch (const RType::EngineException &e) {
        std::cerr << "Erreur : " << e.what() << std::endl;
        return 84;
    }
    return 0;
}
```

Les points importants :

- **La fenêtre est créée en premier.** `Texture2D`, `Font` et `Sound` ont besoin de la fenêtre (et de son périphérique audio) pour se charger. Et comme `addComponent` construit le composant tout de suite, l'ordre des lignes compte.
- **Les entrées ont une entité à elles**, avec un seul `KeyboardManager` et un seul `MouseManager`. Les entités qui réagissent au clavier reçoivent un composant `Keyboard`, un simple lien vers ce `KeyboardManager` (voir [Lire le clavier et la souris](#lire-le-clavier-et-la-souris)).
- **Tous les systèmes sont créés avant la boucle**, jamais pendant `update()` (voir le [piège n°2](ECS_GUIDE.fr.md#piège-n2--créer-un-système-pendant-update)).
- **`WindowShouldClose()`** est la fonction de raylib qui détecte la fermeture de la fenêtre (croix ou `Échap`). `Window` ne la propose pas encore elle-même (voir [section 9](#9-limites-actuelles-du-moteur)).
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
RType::Entity spawnPlayer(RType::GameManager &gm, RType::KeyboardManager &keyboard)
{
    RType::Entity player = gm.createEntity();

    gm.addComponent<Player>(player);
    gm.addComponent<RType::Position2D>(player, 100.f, 300.f);
    gm.addComponent<RType::Velocity2D>(player);
    gm.addComponent<RType::Clock>(player);
    gm.addComponent<Hitbox>(player, 64.f, 32.f);
    gm.addComponent<RType::Texture2D>(player, ASSETS_DIR "player.png");
    gm.addComponent<RType::Sprite2D>(player);
    gm.addComponent<RType::Keyboard>(player, keyboard);
    return player;
}

RType::Entity spawnBullet(RType::GameManager &gm, RType::Position2D pos)
{
    RType::Entity bullet = gm.createEntity();

    gm.addComponent<Bullet>(bullet);
    gm.addComponent<RType::Position2D>(bullet, pos);
    gm.addComponent<RType::Velocity2D>(bullet, 800.f, 0.f);
    gm.addComponent<RType::Clock>(bullet);
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
    // 1. Entrées : lire le clavier et la souris
    gm.createSystem<RType::KeyboardManager>(RType::KeyboardManager::systemKeyboardManager);
    gm.createSystem<RType::MouseManager>(RType::MouseManager::systemMouseManager);

    // 2. Temps : mettre à jour toutes les horloges
    gm.createSystem<RType::Clock>(RType::Clock::systemClock);

    // 3. Logique du jeu
    gm.createSystem<Player, RType::Velocity2D, RType::Keyboard>(steerPlayer);
    gm.createSystem(shoot);
    gm.createSystem<Spawner, RType::Clock>(spawnEnemies);
    gm.createSystem(collisions);
    gm.createSystem(destroyOffScreen);
    gm.createSystem<Score, RType::Text2D>(showScore);

    // 4. Physique : appliquer les vitesses
    gm.createSystem<RType::Velocity2D, RType::Position2D, RType::Clock>(
        RType::Velocity2D::systemVelocity2D);

    // 5. Animation
    gm.createSystem<RType::SpriteSheet2D, RType::Clock>(
        RType::SpriteSheet2D::systemAnimateSpriteSheet2D);

    // 6. Affichage : effacer l'écran, puis dessiner
    gm.createSystem<RType::Window>(RType::Window::systemClearWindow);
    gm.createSystem<RType::Sprite2D, RType::Texture2D, RType::Position2D>(
        RType::Sprite2D::systemDrawSprite2D);
    gm.createSystem<RType::SpriteSheet2D, RType::Texture2D, RType::Position2D>(
        RType::SpriteSheet2D::systemDrawSpriteSheet2D);
    gm.createSystem<RType::Text2D, RType::Font, RType::Position2D>(
        RType::Text2D::systemDrawText2D);

    // 7. Son
    gm.createSystem<RType::Sound>(RType::Sound::systemSound);
}
```

- **Les systèmes du moteur s'enregistrent comme les vôtres** : `gm.createSystem<Composants...>(Classe::systemXxx)`, avec les composants dans le même ordre que les paramètres de la fonction. Les paramètres `const` sont acceptés.
- **`KeyboardManager::systemKeyboardManager` et `Clock::systemClock` doivent s'exécuter exactement une fois par frame**, avant tout système qui lit le clavier ou le temps.
- **L'ordre d'affichage suit l'ordre des systèmes** : les sprites sont dessinés, puis les planches animées, puis les textes par-dessus. À l'intérieur d'un même système, l'ordre des entités n'est pas garanti (voir [Un fond d'écran](#un-fond-décran)).

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
gm.addComponent<RType::Clock>(enemy);
gm.addComponent<RType::Texture2D>(enemy, ASSETS_DIR "enemy.png");

RType::SpriteSheet2D sheet;
sheet.recNb = {1, 4};                               // 1 ligne, 4 colonnes
sheet.frameDuration = std::chrono::milliseconds(80);
gm.addComponent<RType::SpriteSheet2D>(enemy, sheet);
```

- **`recNb` est un `Size2D`, dont l'ordre est `{height, width}`** : `{1, 4}` veut dire 1 ligne et 4 colonnes. Aucune des deux valeurs ne doit valoir 0.
- **L'animation a besoin d'un `Clock` sur la même entité.** `animation = false` fige la case courante, et `currentTile` choisit la case affichée.

### Déplacer une entité

`Velocity2D` déplace la `Position2D` de son entité, en pixels par seconde :

```cpp
gm.addComponent<RType::Position2D>(bullet, 100.f, 300.f);
gm.addComponent<RType::Velocity2D>(bullet, 800.f, 0.f);   // 800 px/s vers la droite
gm.addComponent<RType::Clock>(bullet);
```

- **Il faut un `Clock` sur chaque entité qui bouge**, en plus de `Velocity2D` et de `Position2D`. Sans lui, le système ne voit pas l'entité, et elle reste immobile sans aucune erreur.
- **Le repère est celui de raylib** : `(0, 0)` en haut à gauche, `x` vers la droite, `y` vers le bas.
- **Pour une gravité**, ajoutez `GravityField2D` et son système (avant celui de `Velocity2D`). Ses champs sont `x`, `y` (la direction, vers le bas par défaut) et `g`, l'intensité **en pixels/s²**. La valeur par défaut (`9.80665`) est donc presque invisible à l'écran : comptez plutôt quelques centaines.

```cpp
gm.addComponent<RType::GravityField2D>(player, 0.f, 1.f, 900.f);   // vers le bas, 900 px/s²

gm.createSystem<RType::GravityField2D, RType::Velocity2D, RType::Clock>(
    RType::GravityField2D::systemGravityField2D);
```

- **`Clock::speed`** multiplie le temps vu par l'entité : `0` la fige (mouvement et animation), `2` la fait aller deux fois plus vite. C'est un entier : pas de ralenti.

### Lire le clavier et la souris

Le `KeyboardManager` et le `MouseManager` de l'entité des entrées (voir [section 3](#3-le-main--fenêtre-entrées-boucle)) sont remis à jour à chaque frame par leur système. Ils exposent trois listes :

| Liste | Contenu |
|---|---|
| `keyPressed` / `buttonPressed` | Les touches et boutons enfoncés **pendant cette frame** (un seul tour) |
| `keyDown` / `buttonDown` | Les touches et boutons **maintenus** |
| `keyReleased` / `buttonReleased` | Les touches et boutons relâchés **pendant cette frame** |

Le `MouseManager` donne aussi `getPos()` (la position du curseur, en `Position2D`) et `getWheel()` (la molette).

Les entités qui réagissent aux entrées ne lisent pas l'entité des entrées directement : elles reçoivent un composant **`Keyboard`** (ou **`Mouse`**), qui contient une référence vers le manager.

```cpp
gm.addComponent<RType::Keyboard>(player, gm.getComponent<RType::KeyboardManager>(input));
gm.addComponent<RType::Mouse>(cursor, gm.getComponent<RType::MouseManager>(input));
```

Le membre s'appelle `manager` dans `Keyboard` (`kb.manager.keyDown`) et `mouse` dans `Mouse` (`m.mouse.getPos()`).

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

Un système sur `Player`, `Velocity2D` et `Keyboard` transforme les touches maintenues en vitesse. Le déplacement lui-même est fait par le système de `Velocity2D`.

```cpp
static bool contains(const std::vector<RType::Key::Value> &keys, RType::Key::Value key)
{
    return std::ranges::find(keys, key) != keys.end();
}

void steerPlayer(const Player &player, RType::Velocity2D &vel, const RType::Keyboard &kb)
{
    const auto &down = kb.manager.keyDown;

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

gm.createSystem<Player, RType::Velocity2D, RType::Keyboard>(steerPlayer);
```

### Tirer : créer des entités pendant la partie

Un système par frame parcourt les joueurs avec `getVector()` (voir [Parcourir une vue dans un système](ECS_GUIDE.fr.md#parcourir-une-vue-dans-un-système--getvector)) et crée une balle quand `Espace` vient d'être pressée :

```cpp
void shoot(RType::GameManager &gm)
{
    for (auto &[e, player, pos, kb] :
        gm.view<Player, RType::Position2D, RType::Keyboard>().getVector()) {
        if (contains(kb.manager.keyPressed, RType::Key::Value::Space))
            spawnBullet(gm, {pos.x + 64.f, pos.y + 12.f});
    }
}

gm.createSystem(shoot);
```

On utilise `keyPressed` et non `keyDown` : sinon, une balle partirait à chaque frame tant que la touche est maintenue.

### Faire apparaître des ennemis à intervalle régulier

Une entité « générateur » porte un `Spawner` et un `Clock`. Son système reçoit le `GameManager` en premier paramètre, pour pouvoir créer des entités :

```cpp
void spawnEnemies(RType::GameManager &gm, Spawner &spawner, const RType::Clock &clock)
{
    static std::mt19937 rng(std::random_device {}());
    std::uniform_real_distribution<float> y(0.f, 720.f - 48.f);

    spawner.elapsed += clock.getElapsed();
    if (spawner.elapsed < spawner.every)
        return;
    spawner.elapsed = std::chrono::milliseconds(0);
    spawnEnemy(gm, {1280.f, y(rng)});
}

// dans le main
RType::Entity spawner = gm.createEntity();
gm.addComponent<Spawner>(spawner);
gm.addComponent<RType::Clock>(spawner);

// dans registerSystems
gm.createSystem<Spawner, RType::Clock>(spawnEnemies);
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
gm.addComponent<RType::Text2D>(score, 32.f, "Score : 0");   // taille, texte
```

Pour un texte qui change, un système met à jour `str` à partir des données du jeu :

```cpp
void showScore(const Score &score, RType::Text2D &text)
{
    text.str = "Score : " + std::to_string(score.value);
}

gm.createSystem<Score, RType::Text2D>(showScore);
```

### Jouer un son

`Sound` se pilote par son champ `state`, que `Sound::systemSound` applique à son prochain passage :

```cpp
gm.addComponent<RType::Sound>(player, ASSETS_DIR "shot.wav");          // loop = false par défaut

// dans un système
sound.state = RType::Sound::State::PLAY;     // jouer depuis le début
sound.state = RType::Sound::State::PAUSED;   // mettre en pause
sound.state = RType::Sound::State::START;    // reprendre après une pause
sound.state = RType::Sound::State::STOP;     // arrêter
```

Pour une musique de fond, passez `true` en second argument (`loop`), puis, au démarrage, mettez `state` à `PLAY` juste après l'ajout : `gm.getComponent<RType::Sound>(music).state = RType::Sound::State::PLAY;`.

### Un fond d'écran

À l'intérieur d'un même système, l'ordre de dessin des entités n'est pas garanti : un fond affiché avec `Sprite2D` peut passer devant le vaisseau. La solution est de lui donner **son propre système de dessin**, enregistré avant les autres, et **pas** de composant `Sprite2D` :

```cpp
struct Background {};

void drawBackground(const Background &, const RType::Texture2D &texture,
    const RType::Position2D &pos)
{
    RType::Sprite2D::systemDrawSprite2D(RType::Sprite2D {}, texture, pos);
}

// dans registerSystems, juste après Window::systemClearWindow
gm.createSystem<Background, RType::Texture2D, RType::Position2D>(drawBackground);
```

---

## 7. À faire

- **Créer la fenêtre en premier**, avant tout `Texture2D`, `Font` ou `Sound`.
- **Créer tous les systèmes au démarrage**, dans une seule fonction (`registerSystems`), dans l'ordre entrées → temps → logique → physique → animation → affichage → son ([section 5](#5-lordre-des-systèmes)).
- **Mettre un `Clock` sur chaque entité qui bouge ou qui s'anime.**
- **Écrire de petits composants**, chacun avec un seul rôle, et **utiliser des tags** (structs vides) pour trier les entités. `Bullet` + `DestroyOffScreen` vaut mieux qu'un `Bullet` qui sait tout faire.
- **Créer chaque sorte d'entité dans une fonction `spawnXxx`** : le même code sert au démarrage et en cours de partie.
- **Garder l'état du jeu dans des composants** (score, vies, niveau) : les systèmes le retrouvent avec une vue, et il disparaît avec son entité.
- **Parcourir les vues avec `getVector()` dans les systèmes par frame**, et construire une seule fois une liste utilisée dans une boucle imbriquée ([collisions](#les-collisions)).
- **Garder les `Entity` dont on a besoin plus tard**, et vérifier `gm.hasEntity(e)` avant de s'en servir : une entité peut avoir été détruite entre-temps ([piège n°4](ECS_GUIDE.fr.md#piège-n4--conserver-un-pointeur-ou-une-référence-vers-le-composant-dune-autre-entité), [recette du projectile](ECS_GUIDE.fr.md#un-projectile-qui-vise-une-cible-susceptible-dêtre-détruite)).
- **Attraper `RType::EngineException` dans le `main`** et renvoyer `84`.
- **Passer les chemins des fichiers par `ASSETS_DIR`** ([section 2](#le-cmakeliststxt)).
- **Tester la logique sans fenêtre** : les systèmes qui n'utilisent que des composants du jeu et `engine-system` (collisions, score, destruction) se testent avec un simple `GameManager`, sans raylib, par exemple dans les tests Criterion.
- **Lancer `./script/clang.sh` et `./script/lint.sh` avant de pousser.**

---

## 8. À ne pas faire

- **Ne pas utiliser `EntityManager`, `ComponentManager` ou `SystemManager`** : tout passe par le `GameManager`.
- **Ne pas charger une texture, une police ou un son avant la fenêtre** : le chargement échoue ou plante.
- **Ne pas créer deux `Window`** : le second constructeur lève une `EngineException`. De même, **un seul `KeyboardManager` et un seul `MouseManager`** pour tout le jeu.
- **Ne pas détruire l'entité des entrées** : les composants `Keyboard` et `Mouse` gardent une référence vers ses managers, qui deviendrait invalide.
- **Ne pas créer de système pendant `update()`** ([piège n°2](ECS_GUIDE.fr.md#piège-n2--créer-un-système-pendant-update)) : tous les systèmes au démarrage. Pour désactiver un comportement, retirez un composant ou utilisez un tag ([recette](ECS_GUIDE.fr.md#mettre-une-entité-en-pause-sans-détruire-de-système)).
- **Ne pas appeler `gm.update()` depuis un système** : l'appel ne fait rien.
- **Ne pas lire un composant juste après l'avoir ajouté dans un système** : il n'existe qu'à la fin de la frame ([piège n°7](ECS_GUIDE.fr.md#piège-n7--lire-un-composant-juste-après-lavoir-ajouté-dans-un-système)).
- **Ne pas mettre de logique ni d'appel au `GameManager` dans un composant** : la logique va dans les systèmes ([piège n°1](ECS_GUIDE.fr.md#piège-n1--appeler-le-gamemanager-depuis-le-constructeur-ou-le-destructeur-dun-composant)).
- **Ne pas inverser `Size2D`** : c'est `{height, width}`, pas `{width, height}`.
- **Ne pas oublier le `Clock`** : une entité avec `Velocity2D` mais sans `Clock` ne bouge pas, et rien ne le signale.
- **Ne pas compter sur l'ordre des entités dans un système** : pour un ordre d'affichage, utilisez des systèmes séparés ([fond d'écran](#un-fond-décran)).
- **Ne pas laisser vivre les entités inutiles** : balles hors de l'écran, explosions terminées… Chaque entité oubliée continue d'être mise à jour et dessinée.
- **Ne pas modifier le moteur pour un besoin propre au jeu** : écrivez un composant et un système côté jeu. Si le moteur doit évoluer, voyez-le avec l'équipe moteur.
- **Ne pas lancer `cmake .` à la racine** : utilisez toujours `-B build` (voir le [README](../README.md)).

---

## 9. Limites actuelles du moteur

Ce que le moteur ne fait pas encore, et comment faire en attendant :

| Limite | En attendant |
|---|---|
| `Window` n'indique pas si la fenêtre doit se fermer | Appeler `WindowShouldClose()` de raylib dans la boucle |
| Une texture n'est pas partagée entre entités : chaque `Texture2D` recharge son fichier | Acceptable pour quelques entités par seconde. Si la création des balles devient lente, le signaler à l'équipe moteur |
| Pas d'ordre d'affichage (profondeur) à l'intérieur d'un système | Un système de dessin par « couche » ([fond d'écran](#un-fond-décran)) |
| Pas de collisions | Un composant `Hitbox` et un système côté jeu ([collisions](#les-collisions)) |
| `Clock::speed` est entier | Pas de ralenti ; `0` pour figer, `1` pour la vitesse normale |
| Le réseau (`engine-network`) est vide | — |

---

## 10. Pour aller plus loin

| Document | Contenu |
|---|---|
| [README](../README.md) | Installation, compilation, tests, linter |
| [Guide d'utilisation de l'ECS](ECS_GUIDE.fr.md) | Toute l'API du `GameManager`, en détail |
| ↳ [Les systèmes](ECS_GUIDE.fr.md#4-les-systèmes) | Systèmes par entité, par frame, avec le `GameManager`, `getVector()` |
| ↳ [La frame](ECS_GUIDE.fr.md#5-la-frame--opérations-immédiates-et-différées) | Ce qui est immédiat et ce qui est différé pendant `update()` |
| ↳ [Les pièges à éviter](ECS_GUIDE.fr.md#7-les-pièges-à-éviter) | Les erreurs les plus courantes, avec leur explication |
| ↳ [Recettes](ECS_GUIDE.fr.md#8-recettes) | Durée de vie, pause, projectile à tête chercheuse, collisions |
| ↳ [Aide-mémoire de l'API](ECS_GUIDE.fr.md#9-aide-mémoire-de-lapi) | Toutes les méthodes sur une page |
| [Notes de conception de l'ECS](ECS_DESIGN.fr.md) | Le pourquoi de l'architecture du moteur |
| `Engine/System/include/BuiltIn/`, `Engine/Graphical/include/` | La documentation Doxygen de chaque composant fourni |
| `Games/Poc/` | Les tests de l'ECS : un exemple d'utilisation pour chaque comportement |
