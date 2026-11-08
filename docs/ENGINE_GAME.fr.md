# Lancer un jeu : `game`

> 🌐 **Langue :** Français · [English](ENGINE_GAME.en.md)

`game` est l'exécutable du moteur. Un jeu n'a pas de C++ et ne se compile pas : c'est un dossier qui contient deux choses :

- **un fichier `config.conf`** : la fenêtre, le nombre d'images par seconde, le dossier des scripts et les modules du moteur à charger ;
- **des scripts Lua** : ils créent les entités, leur donnent des composants, enregistrent les systèmes et décident quand le jeu s'arrête.

Le moteur s'occupe du reste : il crée le `GameManager`, charge les modules, lance chaque script, puis fait tourner la boucle.

- **Public visé :** toute personne qui crée un jeu avec le moteur.
- **Pour écrire les scripts**, consultez [le guide du scripting Lua](LUA_GUIDE.fr.md).
- **Pour les écrire plus vite dans VS Code**, installez l'extension **R-Type lua engine epitech**, publiée par Ariel Amriou (identifiant d'éditeur `ariel-amriou`) :

  ```sh
  code --install-extension ariel-amriou.r-type-lua-engine-epitech
  ```
- **Pour l'ECS en détail** (entités, composants, systèmes, vues), consultez [le guide d'utilisation de l'ECS](ECS_GUIDE.fr.md).

> [!TIP]
> Tous les exemples de ce guide ont été lancés avec le moteur actuel, et tous les messages d'erreur de la [section 8](#8-les-erreurs-et-comment-les-régler) ont été reproduits.

---

## Sommaire

1. [Ce que fait le moteur au lancement](#1-ce-que-fait-le-moteur-au-lancement)
2. [Le fichier `config.conf`](#2-le-fichier-configconf)
3. [Les modules](#3-les-modules)
4. [Mettre en place le jeu](#4-mettre-en-place-le-jeu)
5. [Un jeu complet](#5-un-jeu-complet)
6. [Le mode debug](#6-le-mode-debug)
7. [À faire / à ne pas faire](#7-à-faire--à-ne-pas-faire)
8. [Les erreurs et comment les régler](#8-les-erreurs-et-comment-les-régler)

---

## 1. Ce que fait le moteur au lancement

Le `main` se trouve dans [`Engine/Game/src/Main.cpp`](../Engine/Game/src/Main.cpp) et appelle `RType::Game::Runner::run()`. Dans l'ordre, il :

1. lit le fichier `config.conf`, qui doit exister (voir la [section 2](#2-le-fichier-configconf)) ;
2. crée le `GameManager` (en mode debug si le moteur est compilé avec `__DEBUG__`, voir la [section 6](#6-le-mode-debug)) ;
3. charge les modules demandés dans `modules`, plus `Clock`, toujours chargé (voir la [section 3](#3-les-modules)) ;
4. charge tous les fichiers `.lua` du dossier des scripts (`srcDirectory`) et de ses sous-dossiers, **dans l'ordre alphabétique de leur chemin**, et appelle la fonction `start(gm)` de chacun (voir [le guide Lua](LUA_GUIDE.fr.md)) ;
5. fait tourner la boucle, à `windowFPS` images par seconde, **jusqu'à ce qu'un script appelle `gm:stop()`** (voir [le guide Lua](LUA_GUIDE.fr.md#arrêter-le-jeu)) ;
6. si une `RType::EngineException` remonte, du moteur ou d'un script, affiche son message et quitte avec le code `84`.

- **Seul un script arrête le jeu.** Fermer la fenêtre ne suffit pas : un jeu qui ne vérifie pas `gm:shouldClose()` pour appeler `gm:stop()` continue de tourner après un clic sur la croix.
- **Tout est cherché par rapport au dossier depuis lequel le jeu est lancé** : `config.conf`, le dossier des scripts, et les chemins que donnent les scripts (images, polices, sons…).

---

## 2. Le fichier `config.conf`

```
# fenêtre
windowFPS = 60
windowWidth = 1280
windowHeight = 720
windowName = Bounce

srcDirectory = scripts
modules = Window, Keyboard, Mouse, Audio
```

| Clé | Rôle | Valeur par défaut |
|---|---|---|
| `windowFPS` | Images par seconde de la boucle ; un script peut les baisser avec `gm:setFps` | `60` |
| `windowWidth`, `windowHeight` | Taille de la fenêtre, en pixels, utilisée par le module `Window` | `800` × `600` |
| `windowName` | Titre de la fenêtre, utilisé par le module `Window` | `My Game` |
| `srcDirectory` | Le dossier des scripts Lua, par rapport au dossier depuis lequel le jeu est lancé | `.` (le dossier de lancement lui-même) |
| `modules` | Les modules du moteur à charger, séparés par des virgules (voir la [section 3](#3-les-modules)) | aucun (seulement `Clock`) |

- **Le fichier est obligatoire**, même vide : sans lui, le jeu s'arrête au démarrage.
- **Chaque clé est facultative.** Une clé absente garde sa valeur par défaut. Si une clé apparaît deux fois, la dernière l'emporte.
- **Une ligne par réglage**, sous la forme `clé = valeur`. Les espaces autour de la clé, du `=` et de la valeur sont ignorés. Les clés respectent la casse.
- **Les lignes vides et celles qui commencent par `#` sont ignorées.**
- **`windowFPS`, `windowWidth` et `windowHeight`** sont des nombres entiers plus grands que 0.
- **Une ligne invalide arrête le jeu** avant que quoi que ce soit ne tourne, avec le fichier et la ligne (voir la [section 8](#8-les-erreurs-et-comment-les-régler)).

Pour les développeurs du moteur : le fichier remplit la variable `RType::GameConfig`, de type `GameInfo`, déclarée dans [`Game.hpp`](../Engine/Game/include/Game.hpp) et lue par [`ConfigBuilder`](../Engine/Game/include/ConfigBuilder.hpp). Ajouter une clé est expliqué dans [le guide pour étendre le moteur](ENGINE_EXTENDING.fr.md#6-récapitulatif).

---

## 3. Les modules

Un module est une fonctionnalité du moteur que le jeu active par son nom. Chacun crée son entité et enregistre son système, **avant** les scripts : les systèmes des scripts passent donc toujours après eux.

| Module | Ce qu'il crée | A besoin de |
|---|---|---|
| `Window` | une entité avec une `Window` (taille et titre pris dans `config.conf`), et le système qui l'efface et la rafraîchit à chaque frame ; les scripts savent si elle doit se fermer avec `gm:shouldClose()`, et règlent ses images par seconde, le plein écran et le curseur ([régler la fenêtre](LUA_GUIDE.fr.md#régler-la-fenêtre)) | |
| `Keyboard` | une entité avec un `KeyboardManager`, et le système qui lit les touches ; les scripts le lisent avec `gm:keyboard()` | `Window` |
| `Mouse` | une entité avec un `MouseManager`, et le système qui lit la souris ; les scripts la lisent avec `gm:mouse()` | `Window` |
| `Audio` | une entité avec le point qui entend les sons, et le système qui joue les `Sound` ; les scripts déplacent ce point avec `gm:setListenerPosition(x, y)` | |
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
├── game                     # l'exécutable du moteur
└── MyGame/
    ├── config.conf          # srcDirectory = scripts
    ├── scripts/
    │   └── game.lua         # les scripts, chargés dans l'ordre alphabétique
    └── sprites/
        └── ship.png
```

Pas de `CMakeLists.txt`, pas de `.cpp` : le jeu n'est pas compilé.

### Compiler le moteur

```sh
cmake -S . -B build
cmake --build build --target game -j
```

L'exécutable est placé dans `Games/`. Il ne se compile qu'une fois, pour tous les jeux : modifier un script ou `config.conf` ne demande pas de recompiler.

### Lancer le jeu

```sh
cd Games/MyGame && ../game
```

Le jeu doit être **lancé depuis son dossier**, celui qui contient `config.conf`.

---

## 5. Un jeu complet

Un vaisseau qui traverse la fenêtre et rebondit sur ses bords. `Échap` ou le bouton de fermeture quittent le jeu.

```
# config.conf
windowName = Bounce
srcDirectory = scripts
modules = Window
```

```lua
-- scripts/game.lua
local WIDTH, HEIGHT = 800, 600

local function bounce(gm, e, pos, vel)
    if pos.x < 0 or pos.x > WIDTH - 32 then vel.x = -vel.x end
    if pos.y < 0 or pos.y > HEIGHT - 32 then vel.y = -vel.y end
end

local function quit(gm)
    if gm:shouldClose() then gm:stop() end
end

function start(gm)
    local ship = gm:createEntity()
    gm:addComponent(ship, 'Position2D', {x = 100, y = 100})
    gm:addComponent(ship, 'Velocity2D', {x = 250, y = 180})
    gm:addComponent(ship, 'Texture2D', {path = 'sprites/ship.png'})
    gm:addComponent(ship, 'Sprite2D', {scale = 2})

    gm:createSystem(quit)
    gm:createSystem({'Position2D', 'Velocity2D'}, bounce)
    gm:addSystem('MasterVelocity2D')
    gm:addSystem('DrawSprite2D')
end
```

- **`config.conf`** n'active que la fenêtre : la taille par défaut, 800 × 600, convient. Le module `Window` efface déjà la fenêtre à chaque frame.
- **`start`** tourne une fois : il crée le vaisseau, puis enregistre quatre systèmes, qui tournent à chaque frame dans cet ordre. `quit` et `bounce` sont propres au jeu : `quit` arrête le jeu avec `gm:stop()` quand la fenêtre doit se fermer ; `MasterVelocity2D` (mouvement) et `DrawSprite2D` (dessin) viennent du moteur.

Tout ce qu'un script peut faire est dans [le guide du scripting Lua](LUA_GUIDE.fr.md).

---

## 6. Le mode debug

Quand le moteur est compilé avec la macro `__DEBUG__`, le jeu tourne en mode debug :

```sh
cmake -S . -B build -DCMAKE_CXX_FLAGS="-D__DEBUG__"
cmake --build build --target game -j
```

**Chaque opération du `GameManager` est écrite dans un fichier de log**, `log/<date>_<heure>.log`, dans le dossier depuis lequel le jeu est lancé :

```
[GameManager] debug mode enabled
[GameManager] createEntity -> entity 0
[GameManager] addComponent<RType::Window> on entity 0
[GameManager] createSystem with ( RType::Window ) -> system 0
```

**Le terminal devient une console** : pendant que le jeu tourne, il affiche une invite `>` et exécute une commande par ligne, au début de la frame suivante.

| Commande | Effet |
|---|---|
| `help` | liste les commandes et leurs arguments |
| `createEntity` | crée une entité et l'affiche sous la forme `#<index> (<génération>)` |
| `destroyEntity <index> <generation>` | détruit une entité ; l'entité `0 0` (la fenêtre) est refusée |
| `addComponent <index> <generation> <component> [{champs}]` | ajoute un composant, avec la même table que `gm:addComponent` : `addComponent 2 0 Position2D {x = 10}` |
| `removeComponent <index> <generation> <component>` | enlève un composant |
| `enableSystem <system>` | ajoute un des [systèmes du moteur](LUA_GUIDE.fr.md#les-systèmes-du-moteur) et affiche son id |
| `disableSystem <id>` | arrête un système, à partir de son id |
| `pause` | met le jeu en pause, ou le relance : pendant la pause, aucun système ne tourne, et les horloges ne comptent pas le temps passé en pause |
| `exit` | arrête le jeu, comme `gm:stop()` |
| `exitConsole` | ferme la console, et laisse le jeu tourner |

- Une commande à laquelle il manque des arguments affiche son usage, et une commande inconnue `Unknown command: <nom>`.
- Une erreur dans une commande (un composant inconnu, un mauvais champ…) est affichée, et le jeu continue de tourner.

---

## 7. À faire / à ne pas faire

### À faire

- **Mettre `Window` dans `modules`** dès que le jeu dessine quelque chose, lit le clavier ou la souris, ou appelle une de ses fonctions `gm:`, comme `gm:shouldClose()`.
- **Avoir un système qui appelle `gm:stop()`** quand `gm:shouldClose()` renvoie `true` : sans lui, la fenêtre ne se ferme pas.
- **Lancer le jeu depuis son dossier**, celui qui contient `config.conf`.
- **Écrire les chemins des scripts par rapport à ce dossier.**

### À ne pas faire

- **Ne pas mettre `Clock` dans `modules`** : il est toujours chargé.
- **Ne pas lancer le jeu depuis un autre dossier** : le moteur n'y trouve ni `config.conf`, ni les scripts.

---

## 8. Les erreurs et comment les régler

Chaque erreur arrête le jeu avec le code de sortie `84`, avant que quoi que ce soit ne tourne.

### `Engine Error: Config error: ...`

```
Engine Error: Config error: config.conf:2: unknown key 'fps'
Engine Error: Config error: config.conf:1: windowWidth expects a number greater than 0, got 'abc'
Engine Error: Config error: config.conf:4: expected 'key = value'
Engine Error: Config error: config.conf: cannot be opened
```

**Cause :** une ligne de `config.conf` a une clé qui n'existe pas, une valeur invalide, ou pas de `=`, et le message donne la ligne. Ou bien le fichier n'est pas dans le dossier depuis lequel le jeu est lancé.
**Solution :** corriger la ligne, avec les clés et les valeurs de la [section 2](#2-le-fichier-configconf). Ou bien lancer le jeu depuis son dossier, ou y créer `config.conf`, même vide.

### `Engine Error: No such module: ...`

```
Engine Error: No such module: Windw
```

**Cause :** un nom de `modules` n'est pas un des modules de la [section 3](#3-les-modules), souvent à cause d'une faute de frappe.
**Solution :** corriger le nom. Les noms de modules respectent la casse.

### `Engine Error: Module ... requires module ...`

```
Engine Error: Module Keyboard requires module Window
```

**Cause :** un module est cité sans le module dont il a besoin.
**Solution :** ajouter le module manquant dans `modules`.

### `Engine Error: Script error: ...`

**Cause :** un script contient une erreur, ou appelle le moteur avec de mauvais arguments. Le message donne le fichier et la ligne.
**Solution :** voir [les erreurs du guide Lua](LUA_GUIDE.fr.md#9-les-erreurs-et-comment-les-régler).

### Rien ne se passe : aucun script ne tourne

**Cause :** `srcDirectory` ne désigne pas le dossier des scripts. Le moteur ne trouve aucun script, et fait tourner une boucle vide sans erreur.
**Solution :** vérifier `srcDirectory` dans `config.conf`, écrit par rapport au dossier depuis lequel le jeu est lancé.

### La fenêtre ne s'ouvre pas

**Cause :** `Window` n'est pas dans `modules`.
**Solution :** ajouter `Window` dans `modules`.

### La fenêtre ne se ferme pas

**Cause :** aucun script n'appelle `gm:stop()` : la croix et `Échap` ne font rien d'autre que rendre `gm:shouldClose()` vrai.
**Solution :** ajouter le système `quit` du [jeu complet](#5-un-jeu-complet).
