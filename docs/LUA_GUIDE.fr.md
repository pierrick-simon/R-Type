# Écrire un jeu en Lua

> 🌐 **Langue :** Français · [English](LUA_GUIDE.en.md)

Avec le moteur, le contenu d'un jeu s'écrit en Lua : les scripts créent les entités, leur donnent les composants du moteur, et enregistrent les systèmes qui tournent à chaque frame. Le jeu n'a pas de C++ : ses réglages sont dans son fichier `config.conf`.

- **Public visé :** toute personne qui écrit les scripts d'un jeu, et, dans la [section 10](#10-pour-les-développeurs-du-moteur--exposer-un-composant-ou-un-système), les développeurs du moteur qui rendent un nouveau composant ou système utilisable depuis Lua.
- **Pour mettre en place le jeu** (`config.conf`, modules, dossier, lancement), consultez [le guide pour lancer un jeu](ENGINE_GAME.fr.md).
- **Pour écrire les scripts plus vite**, une extension VS Code pour le Lua du moteur est disponible : **R-Type lua engine epitech**, publiée par Ariel Amriou (identifiant d'éditeur `ariel-amriou`). Installez-la avec :

  ```sh
  code --install-extension ariel-amriou.r-type-lua-engine-epitech
  ```

  ou cherchez « R-Type lua engine epitech » dans la Marketplace de VS Code (vue Extensions, `Ctrl+Shift+X`).

> [!TIP]
> Tous les exemples de ce guide ont été lancés avec le moteur actuel, et tous les messages d'erreur de la [section 9](#9-les-erreurs-et-comment-les-régler) ont été reproduits.

---

## Sommaire

1. [Un script](#1-un-script)
2. [Les entités](#2-les-entités)
3. [Les composants](#3-les-composants)
4. [Les systèmes](#4-les-systèmes)
5. [Les vues : parcourir des entités n'importe où](#5-les-vues--parcourir-des-entités-nimporte-où)
6. [Ce qui se passe pendant une frame](#6-ce-qui-se-passe-pendant-une-frame)
7. [Un jeu complet](#7-un-jeu-complet)
8. [À faire / à ne pas faire](#8-à-faire--à-ne-pas-faire)
9. [Les erreurs et comment les régler](#9-les-erreurs-et-comment-les-régler)
10. [Pour les développeurs du moteur : exposer un composant ou un système](#10-pour-les-développeurs-du-moteur--exposer-un-composant-ou-un-système)
11. [Aide-mémoire de l'API](#11-aide-mémoire-de-lapi)

---

## 1. Un script

Le moteur charge tous les fichiers `.lua` du dossier des scripts du jeu (`srcDirectory` dans [`config.conf`](ENGINE_GAME.fr.md#2-le-fichier-configconf)) et de ses sous-dossiers, dans l'ordre alphabétique de leur chemin, puis appelle la fonction `start` de chacun, **une seule fois**, avec le `GameManager` :

```lua
function start(gm)
    print('le jeu démarre')
end
```

- **Chaque script doit définir `start`.** Un script qui n'en a pas arrête le jeu (voir la [section 9](#9-les-erreurs-et-comment-les-régler)).
- **`start` crée les choses ; les systèmes les font tourner.** Ce qui doit se passer à chaque frame va dans un système, enregistré depuis `start` ([section 4](#4-les-systèmes)).
- **Chaque script a ses propres globales.** Une variable ou une fonction globale définie dans `a.lua` n'est pas visible depuis `b.lua` : deux scripts peuvent donc tous les deux appeler leur fonction `process` sans conflit. Utilisez quand même `local` pour tout le reste.
- **La bibliothèque de base de Lua et `math` sont disponibles** : `print`, `pairs`, `ipairs`, `pcall`, `error`, `type`, `tostring`, `tonumber`…, et `math.random`, `math.sqrt`, `math.abs`, `math.floor`, `math.min`, `math.max`, `math.pi`…
- **Les chemins** (images, polices, modèles, sons) sont relatifs au dossier depuis lequel le jeu est lancé, pas au dossier des scripts.

---

## 2. Les entités

```lua
local e = gm:createEntity()     -- une nouvelle entité
gm:hasEntity(e)                 -- true
gm:destroyEntity(e)             -- la détruit, avec tous ses composants
gm:hasEntity(e)                 -- false
print(e)                        -- #1 (0) : son index, puis sa génération
```

Une entité n'est qu'une poignée. Les entités se comparent avec `==`. La poignée d'une entité détruite n'est jamais confondue avec une nouvelle entité qui réutilise son index.

### Garder des données sur une entité

Un script ne crée pas de composants à lui : les données du jeu (le joueur, le score, les points de vie de chaque ennemi…) se gardent dans des variables `local` du script. Pour des données par entité, une table dont la clé est `tostring(e)` :

```lua
local hp = {}                         -- les points de vie, par entité

local function hurt(gm, e, pos)
    local key = tostring(e)

    if hp[key] then
        hp[key] = hp[key] - 1
        if hp[key] == 0 then
            gm:destroyEntity(e)
            hp[key] = nil             -- l'entité n'existe plus
        end
    end
end

function start(gm)
    local enemy = gm:createEntity()
    gm:addComponent(enemy, 'Position2D')
    hp[tostring(enemy)] = 3
    gm:createSystem({'Position2D'}, hurt)
end
```

- **La clé est `tostring(e)`, pas `e`.** L'entité qu'un système ou une vue donne est un autre objet Lua que celle gardée par le script, même si `==` les dit égales : `hp[e]` y donnerait `nil`. `tostring(e)` est le même texte pour les deux.
- **Enlever la clé quand l'entité est détruite**, pour que la table ne garde pas des entités qui n'existent plus.

---

## 3. Les composants

### Ajouter un composant

```lua
gm:addComponent(e, 'Position2D', {x = 100, y = 50})
gm:addComponent(e, 'Velocity2D', {x = 250})     -- y garde sa valeur par défaut, 0
gm:addComponent(e, 'Clock')                     -- tous les champs gardent leur valeur par défaut
```

La table est facultative. **Les champs omis gardent leur valeur par défaut.** Un champ qui n'existe pas, ou une valeur du mauvais type, s'arrête avec un message clair :

```
Invalid arguments for Position2D: no field 'z'
Invalid arguments for Position2D: field 'x' expects a number, got a string
```

### Lire et modifier un composant

```lua
gm:hasComponent(e, 'Position2D')            -- true / false, jamais d'erreur
local pos = gm:getComponent(e, 'Position2D')
pos.x = 42                                  -- modifie le vrai composant
gm:removeComponent(e, 'Position2D')
gm:countComponents('Position2D')            -- nombre d'entités qui en ont un
```

`getComponent` donne **le vrai composant, pas une copie** : écrire ses champs le modifie pour tout le moteur, y compris les systèmes C++.

### Les composants

| Nom | Champs (valeur par défaut) | Rôle |
|---|---|---|
| `Position2D` | `x` (0), `y` (0) | position en pixels, (0, 0) dans le coin en haut à gauche |
| `Velocity2D` | `x` (0), `y` (0) | vitesse en pixels par seconde |
| `Origin2D` | `x` (0), `y` (0) | point d'origine |
| `Size2D` | `width` (0), `height` (0) | taille en pixels |
| `Friction2D` | `f` (0) | ralentit une `Velocity2D` |
| `GravityField2D` | `x` (0), `y` (1), `g` (9.80665) | attire une `Velocity2D` dans une direction |
| `Rotation2D` | `angle` (0) | orientation, en radians |
| `AngularVelocity2D` | `w` (0) | fait tourner une `Rotation2D`, en radians par seconde |
| `Clock` | `speed` (1), méthode `elapsed()` | l'horloge propre à une entité |
| `Texture2D` | `path` (**obligatoire**) | une image, chargée depuis `path` |
| `Sprite2D` | `scale` (1), `rotation` (0), `flip` (false) | dessine la `Texture2D` de l'entité |
| `SpriteSheet2D` | `rows` (1), `columns` (1), `currentTile` (0), `frameDuration` (100, en ms), `animation` (true), `scale` (1), `rotation` (0), `flip` (false) | dessine et anime une case de la `Texture2D` à la fois |
| `Font` | `path` (**obligatoire**) | une police, chargée depuis `path` |
| `Text2D` | `str` ("Hello World"), `fontSize` (10), `rotation` (0), `spacing` (0) | dessine un texte avec la `Font` de l'entité |
| `Position3D` | `x` (0), `y` (0), `z` (0) | position dans l'espace 3D |
| `Velocity3D` | `x` (0), `y` (0), `z` (0) | vitesse en unités par seconde |
| `Friction3D` | `f` (0) | ralentit une `Velocity3D` |
| `GravityField3D` | `x` (0), `y` (-1), `z` (0), `g` (9.80665) | attire une `Velocity3D` dans une direction |
| `Rotation3D` | `x` (0), `y` (0), `z` (0), `w` (1) | orientation, sous forme de quaternion |
| `AngularVelocity3D` | `x` (0), `y` (0), `z` (0) | fait tourner une `Rotation3D` autour de chaque axe, en radians par seconde |
| `Model3D` | `path` (**obligatoire**) | un modèle 3D, chargé depuis `path` |
| `ModelAnimation3D` | `path` (**obligatoire**) | les animations d'un modèle 3D |
| `Sprite3D` | `scale` (1) | dessine le `Model3D` de l'entité |
| `SpriteSheet3D` | `index` (0), `frameDuration` (100, en ms), `animation` (true), `scale` (1) | joue une animation du `Model3D` |
| `Camera3D` | `targetX` (0), `targetY` (0.4), `targetZ` (0), `upX` (0), `upY` (1), `upZ` (0), `fovy` (45), `projection` (`CameraProjection.PERSPECTIVE`), `updateMode` (`CameraMode.CUSTOM`) | la caméra 3D, placée à la `Position3D` de l'entité et tournée vers le point visé (voir [la caméra](#la-caméra)) |
| `CameraAngle` | `yaw` (0), `pitch` (0), `minPitch` (-89°), `maxPitch` (89°), en radians | oriente la `Camera3D` depuis sa `Position3D` selon ces angles |
| `CameraAngleInput` | `yaw` (0), `pitch` (0), en radians | le virage à ajouter au `CameraAngle` pendant cette frame |
| `CameraMouseControl` | `sensitivity` (0.0025), `invertY` (false) | transforme le mouvement de la souris en `CameraAngleInput` |
| `Sound` | `path` (**obligatoire**), `state` (`SoundState.STOP`), `loop` (false), `stereo` (false), `attenuation` (`Attenuation.INVERSE`), `minDistance` (1), `maxDistance` (très grande), méthodes `setVolume(v)` et `setPitch(p)` | un son, chargé depuis `path` (voir [les sons](#les-sons)) |

Les cinq composants marqués **obligatoire** sont construits à partir d'un fichier, puis reçoivent les autres champs de la table : `gm:addComponent(e, 'Texture2D', {path = 'sprites/ship.png'})`.

---

## 4. Les systèmes

Un système est une fonction que le moteur appelle **à chaque frame**. Les systèmes tournent **dans l'ordre de leur création** : d'abord ceux des modules (fenêtre, clavier, horloge), puis ceux des scripts, dans l'ordre des scripts et des appels.

### Les systèmes propres au jeu

**Une fois par frame :**

```lua
local frame = 0

function process(gm)
    frame = frame + 1
end

function start(gm)
    gm:createSystem(process)
end
```

**Une fois par entité qui a certains composants :**

```lua
function process(gm, e, pos, vel)
    if pos.x > 800 then vel.x = -vel.x end
end

function start(gm)
    gm:createSystem({'Position2D', 'Velocity2D'}, process)
end
```

- La fonction reçoit le `GameManager`, l'entité, puis **un composant par nom, dans l'ordre de la liste**. `{'Velocity2D', 'Position2D'}` donne `process(gm, e, vel, pos)`.
- Seules les entités qui ont **tous** les composants sont données. Si l'un d'eux n'a jamais été ajouté, la fonction n'est simplement pas appelée.
- Ce sont les vrais composants : écrire leurs champs les modifie.
- Le système peut être créé **avant** les entités : il les trouve dès qu'elles ont les composants.
- Un mauvais nom arrête le jeu **au `start`**, pas à la première frame.
- Une liste vide, `{}`, revient à ne pas donner de liste : une fois par frame.

### Les systèmes du moteur

`gm:addSystem(nom)` ajoute un système écrit en C++ par le moteur. Il agit sur les entités qui ont les composants indiqués dans le tableau.

| Nom | Agit sur les entités qui ont | Rôle |
|---|---|---|
| `MasterVelocity2D` | `Position2D`, `Velocity2D` | les déplace, avec l'horloge du jeu |
| `Velocity2D` | `Velocity2D`, `Position2D`, `Clock` | les déplace, avec leur propre horloge |
| `MasterFriction2D` | `Velocity2D`, `Friction2D` | les ralentit, avec l'horloge du jeu |
| `Friction2D` | `Friction2D`, `Velocity2D`, `Clock` | les ralentit, avec leur propre horloge |
| `MasterGravityField2D` | `Velocity2D`, `GravityField2D` | applique la gravité, avec l'horloge du jeu |
| `GravityField2D` | `GravityField2D`, `Velocity2D`, `Clock` | applique la gravité, avec leur propre horloge |
| `MasterAngularVelocity2D` | `Rotation2D`, `AngularVelocity2D` | les fait tourner, avec l'horloge du jeu |
| `AngularVelocity2D` | `AngularVelocity2D`, `Rotation2D`, `Clock` | les fait tourner, avec leur propre horloge |
| `MasterVelocity3D` | `Position3D`, `Velocity3D` | les déplace, avec l'horloge du jeu |
| `Velocity3D` | `Velocity3D`, `Position3D`, `Clock` | les déplace, avec leur propre horloge |
| `MasterFriction3D` | `Velocity3D`, `Friction3D` | les ralentit, avec l'horloge du jeu |
| `Friction3D` | `Friction3D`, `Velocity3D`, `Clock` | les ralentit, avec leur propre horloge |
| `MasterGravityField3D` | `Velocity3D`, `GravityField3D` | applique la gravité, avec l'horloge du jeu |
| `GravityField3D` | `GravityField3D`, `Velocity3D`, `Clock` | applique la gravité, avec leur propre horloge |
| `MasterAngularVelocity3D` | `Rotation3D`, `AngularVelocity3D` | les fait tourner, avec l'horloge du jeu |
| `AngularVelocity3D` | `AngularVelocity3D`, `Rotation3D`, `Clock` | les fait tourner, avec leur propre horloge |
| `Clock` | `Clock` | fait avancer leur propre horloge |
| `DrawSprite2D` | `Sprite2D`, `Texture2D`, `Position2D` | les dessine |
| `DrawSpriteSheet2D` | `SpriteSheet2D`, `Texture2D`, `Position2D` | dessine leur case actuelle |
| `MasterAnimateSpriteSheet2D` | `SpriteSheet2D` | les anime, avec l'horloge du jeu |
| `AnimateSpriteSheet2D` | `SpriteSheet2D`, `Clock` | les anime, avec leur propre horloge |
| `DrawText2D` | `Text2D`, `Font`, `Position2D` | dessine leur texte |
| `DrawSprite3D` | `Sprite3D`, `Model3D`, `Position3D` | les dessine en 3D |
| `DrawSpriteSheet3D` | `SpriteSheet3D`, `Model3D`, `Position3D` | les dessine en 3D, animés |
| `MasterAnimateSpriteSheet3D` | `SpriteSheet3D`, `Model3D`, `ModelAnimation3D` | les anime, avec l'horloge du jeu |
| `AnimateSpriteSheet3D` | `SpriteSheet3D`, `Model3D`, `ModelAnimation3D`, `Clock` | les anime, avec leur propre horloge |
| `UpdateCamera` | `Camera3D`, `Position3D` | place la caméra à la `Position3D` de son entité |
| `Draw3D` | `Camera3D` | dessine les `Sprite3D` et `SpriteSheet3D` tels que la caméra les voit |
| `CameraMouseControl` | la souris | ajoute le mouvement de la souris au `CameraAngleInput` des entités qui ont un `CameraMouseControl` |
| `CameraAngleInput` | `CameraAngle`, `CameraAngleInput` | ajoute l'entrée à l'angle, puis vide l'entrée |
| `CameraAngle` | `Camera3D`, `Position3D`, `CameraAngle` | oriente la caméra selon son angle |
| `Sound2D` | `Sound`, `Position2D` | place les sons `stereo` à la position de leur entité |
| `KeyboardManagerClear` | le clavier | vide les touches lues pendant la frame |

Les systèmes `Master…` utilisent l'horloge du jeu, que le moteur fait toujours tourner : c'est le choix le plus simple. Les autres utilisent une `Clock` donnée à chaque entité, pour ralentir ou accélérer certaines entités seulement (avec son `speed`), et ont aussi besoin du système `Clock`.

Les systèmes de la fenêtre, du clavier, de la souris, du son et de l'horloge du jeu sont enregistrés par les [modules](ENGINE_GAME.fr.md#3-les-modules), une seule fois : ils ne sont pas dans ce tableau, pour qu'aucun script ne les fasse tourner deux fois par frame. `UpdateKeybind` n'est pas non plus dans ce tableau : il agit sur un composant que les scripts ne créent pas.

### L'ordre des systèmes

L'ordre habituel, dans `start` :

1. la logique du jeu (lecture du clavier, collisions, règles) ;
2. le mouvement (`MasterVelocity2D`…) ;
3. le dessin (`DrawSprite2D`, `DrawText2D`…). La fenêtre est déjà effacée par le module `Window`, dont le système tourne en premier.

### Arrêter un système

`gm:createSystem` et `gm:addSystem` renvoient l'id du système créé. `gm:destroySystem(id)` l'arrête, et `gm:hasSystem(id)` dit s'il existe encore :

```lua
local intro
local frame = 0

function showIntro(gm)
    frame = frame + 1
    if frame == 180 then
        gm:destroySystem(intro)       -- l'intro s'arrête après 180 frames
    end
end

function start(gm)
    intro = gm:createSystem(showIntro)
end
```

- Un système peut s'arrêter lui-même, ou en arrêter un autre, depuis un système : il tourne encore jusqu'à la fin de la frame ([section 6](#6-ce-qui-se-passe-pendant-une-frame)).
- Arrêter un système déjà arrêté ne fait rien.

### Arrêter le jeu

`gm:stop()` arrête le jeu : la frame en cours va jusqu'au bout, puis la boucle s'arrête et le jeu quitte avec le code de sortie `0`. **C'est la seule façon d'arrêter le jeu** : tant qu'aucun script ne l'appelle, il tourne, même si on ferme la fenêtre.

```lua
local function quit(gm)
    if gm:shouldClose() then
        gm:stop()                     -- la croix ou Échap quittent le jeu
    end
end

function start(gm)
    gm:createSystem(quit)
end
```

- Les systèmes qui suivent dans la même frame tournent encore, et les changements de la frame sont appliqués.
- Appeler `gm:stop()` plusieurs fois ne fait rien de plus.
- `gm:shouldClose()` renvoie `true` quand le bouton de fermeture de la fenêtre ou `Échap` a été pressé. Il a besoin du module `Window`.
- Un jeu qui veut quitter avec une autre touche lit le clavier ([plus bas](#lire-le-clavier-et-la-souris)) et appelle `gm:stop()` de la même façon.

### Régler la fenêtre

La fenêtre du module `Window` a trois réglages, qu'un script lit et change avec des fonctions `gm:` :

```lua
local function fullScreen(gm)
    if gm:keyboard():isPressed(Key.F11) then
        gm:setFullScreen(not gm:fullScreen())   -- F11 passe en plein écran et en sort
    end
end

function start(gm)
    gm:setHideCursor(true)
    gm:setFps(30)
    gm:createSystem(fullScreen)
end
```

| Lire | Changer | Réglage | Par défaut |
|---|---|---|---|
| `gm:fps()` | `gm:setFps(n)` | les images par seconde de la fenêtre | `60` |
| `gm:fullScreen()` | `gm:setFullScreen(b)` | si la fenêtre est en plein écran | `false` |
| `gm:hideCursor()` | `gm:setHideCursor(b)` | si le curseur de la souris est caché et gardé dans la fenêtre | `false` |

- Un changement est appliqué au début de la frame suivante : celui fait dans `start` est là dès la première frame.
- `gm:setFps` peut ralentir le jeu, pas l'accélérer : la boucle ne va jamais plus vite que `windowFPS` de [`config.conf`](ENGINE_GAME.fr.md#2-le-fichier-configconf).
- Ces fonctions ont besoin du module `Window`.

### Lire le clavier et la souris

`gm:keyboard()` donne le clavier du module `Keyboard`, et `gm:mouse()` la souris du module `Mouse` (voir [les modules](ENGINE_GAME.fr.md#3-les-modules)). Le moteur les met à jour au début de chaque frame : un système les lit à chaque passage.

```lua
function steer(gm, e, pos)
    local kb = gm:keyboard()

    if kb:isDown(Key.Left) then pos.x = pos.x - 5 end
    if kb:isDown(Key.Right) then pos.x = pos.x + 5 end
end

function shoot(gm)
    local mouse = gm:mouse()

    if mouse:isPressed(MouseButton.Left) then
        print('tir vers', mouse.x, mouse.y)
    end
end
```

| Appel | Vrai si |
|---|---|
| `kb:isPressed(touche)` / `mouse:isPressed(bouton)` | la touche ou le bouton a été appuyé **pendant cette frame** |
| `kb:isDown(touche)` / `mouse:isDown(bouton)` | la touche ou le bouton est **tenu** |
| `kb:isReleased(touche)` / `mouse:isReleased(bouton)` | la touche ou le bouton a été relâché **pendant cette frame** |

- `mouse.x` et `mouse.y` donnent la position du curseur dans la fenêtre, et `mouse.wheel` le mouvement de la molette. Ils sont en lecture seule.
- Les touches sont dans la table `Key` : les lettres (`Key.A` à `Key.Z`), les chiffres (`Key.Num0` à `Key.Num9`), les flèches (`Key.Left`, `Key.Right`, `Key.Up`, `Key.Down`), `Key.Space`, `Key.Enter`, `Key.Escape`, `Key.Tab`, `Key.Backspace`, `Key.LShift`, `Key.LControl`, `Key.LAlt`, le pavé numérique (`Key.Numpad0` à `Key.Numpad9`) et les touches de fonction (`Key.F1` à `Key.F15`).
- Les boutons sont dans la table `MouseButton` : `Left`, `Right`, `Middle`, `Side`, `Extra`, `Forward`, `Back`.
- Le clavier et la souris appartiennent à leur module : un script ne peut pas en créer d'autres.

### Le temps

`gm:elapsed()` donne le temps écoulé pendant **la frame précédente**, en millisecondes, mesuré par l'horloge du jeu. Multiplier une vitesse par ce temps fait avancer les choses à la même vitesse, quel que soit le nombre de FPS :

```lua
function move(gm, e, pos)
    pos.x = pos.x + 200 * gm:elapsed() / 1000      -- 200 pixels par seconde
end
```

Pour un chrono ou une action répétée, il suffit d'additionner ce temps :

```lua
local timer = 0

function spawn(gm)
    timer = timer + gm:elapsed()
    if timer >= 2000 then                           -- toutes les 2 secondes
        timer = timer - 2000
        gm:addComponent(gm:createEntity(), 'Position2D', {x = math.random(0, 800)})
    end
end
```

Une entité qui a sa propre `Clock` lit son temps avec `gm:getComponent(e, 'Clock'):elapsed()`. Il est multiplié par son `speed` et n'avance qu'avec le système `Clock`. L'horloge du jeu appartient au moteur : un script ne peut pas en créer une autre.

### Les sons

Les sons demandent le module `Audio` (voir [les modules](ENGINE_GAME.fr.md#3-les-modules)) : il crée le point qui entend les sons et le système qui les joue. Un son se charge une fois, dans `start`, puis se joue autant de fois que voulu :

```lua
local shot

function shoot(gm)
    if gm:keyboard():isPressed(Key.Space) then
        gm:getComponent(shot, 'Sound').state = SoundState.PLAY
    end
end

function start(gm)
    shot = gm:createEntity()
    gm:addComponent(shot, 'Sound', {path = 'sounds/shot.mp3'})
    gm:createSystem(shoot)
end
```

| `state` | Effet |
|---|---|
| `SoundState.PLAY` | joue le son depuis le début |
| `SoundState.START` | reprend le son là où il s'était arrêté |
| `SoundState.PAUSED` | met le son en pause |
| `SoundState.STOP` | arrête le son et le remet au début |
| `SoundState.PLAYING` | le son est en train de jouer : le moteur le met lui-même, puis remet `STOP` à la fin du son |

- `loop = true` rejoue le son en boucle, par exemple pour une musique.
- `sound:setVolume(v)` règle le volume (`1` par défaut) et `sound:setPitch(p)` la hauteur (`1` par défaut).
- Un son `stereo = true` est placé dans l'espace : avec le système `Sound2D`, il suit la `Position2D` de son entité, et son volume baisse avec la distance selon `attenuation` (`Attenuation.NONE`, `INVERSE`, `LINEAR` ou `EXPONENTIAL`), entre `minDistance` et `maxDistance`.
- Le point qui entend les sons appartient au module : un script ne peut pas en créer d'autre. Il est en (0, 0), et `gm:setListenerPosition(x, y)` le déplace, par exemple pour suivre le joueur.

### La caméra

Un jeu en 3D dessine à travers une `Camera3D`. La caméra se tient à la `Position3D` de son entité et regarde son point visé (`targetX`, `targetY`, `targetZ`) :

```lua
local camera

local function move(gm)
    local pos = gm:getComponent(camera, 'Position3D')
    pos.x = pos.x + 0.01            -- la caméra glisse vers la droite
end

function start(gm)
    camera = gm:createEntity()
    gm:addComponent(camera, 'Position3D', {x = 0, y = 2, z = 6})
    gm:addComponent(camera, 'Camera3D', {targetY = 0, fovy = 60})

    gm:createSystem(move)
    gm:addSystem('UpdateCamera')
    gm:addSystem('Draw3D')
end
```

- `UpdateCamera` place la caméra à sa `Position3D` à chaque frame, et `Draw3D` dessine les `Sprite3D` et `SpriteSheet3D` tels que la caméra les voit.
- `upX`, `upY`, `upZ` donnent la direction du haut de la caméra, et `fovy` son champ de vision vertical, en degrés.
- `projection` vaut `CameraProjection.PERSPECTIVE` ou `CameraProjection.ORTHOGRAPHIC`.
- `updateMode` vaut `CameraMode.CUSTOM` par défaut : la caméra ne bouge qu'avec sa `Position3D` et son point visé. `FREE`, `ORBITAL`, `FIRST_PERSON` et `THIRD_PERSON` sont les modes de caméra de raylib.
- Un jeu n'a qu'une caméra : une deuxième `Camera3D` arrête le jeu avec `Camera can be created only once`.

Pour tourner la caméra avec la souris, comme dans un jeu à la première personne, on lui donne un `CameraAngle`, un `CameraAngleInput` et un `CameraMouseControl`. Il faut le module `Mouse` :

```lua
function start(gm)
    local camera = gm:createEntity()
    gm:addComponent(camera, 'Position3D', {y = 2, z = 6})
    gm:addComponent(camera, 'Camera3D')
    gm:addComponent(camera, 'CameraAngle')
    gm:addComponent(camera, 'CameraAngleInput')
    gm:addComponent(camera, 'CameraMouseControl', {sensitivity = 0.003})

    gm:setHideCursor(true)              -- le curseur reste dans la fenêtre
    gm:addSystem('CameraMouseControl')  -- souris -> CameraAngleInput
    gm:addSystem('CameraAngleInput')    -- CameraAngleInput -> CameraAngle
    gm:addSystem('UpdateCamera')
    gm:addSystem('CameraAngle')         -- CameraAngle -> là où regarde la caméra
    gm:addSystem('Draw3D')
end
```

- `CameraAngle` remplace les champs du point visé : la caméra regarde depuis sa `Position3D` selon `yaw` (gauche et droite) et `pitch` (haut et bas), et `pitch` reste entre `minPitch` et `maxPitch`.
- Un script peut tourner la caméra sans la souris en ajoutant à `CameraAngleInput`, par exemple avec les flèches.

---

## 5. Les vues : parcourir des entités n'importe où

`gm:view` parcourt les entités qui ont certains composants, **depuis `start` ou depuis un système** :

```lua
for e, pos, vel in gm:view({'Position2D', 'Velocity2D'}) do
    pos.x = pos.x + vel.x
end
```

- Mêmes règles qu'un système : l'entité, puis un composant par nom dans l'ordre de la liste, les vrais composants, un mauvais nom arrête avec une erreur.
- `break` et `return` marchent comme dans n'importe quelle boucle.
- **La boucle peut détruire des entités**, même dans `start` : une entité détruite ou modifiée pendant la boucle, avant son tour, est sautée.
- Dans un système, elle croise deux groupes d'entités, par exemple tout ce qui bouge contre chaque obstacle :

```lua
function process(gm)
    for mover, mpos, vel in gm:view({'Position2D', 'Velocity2D'}) do
        for obstacle, opos, size in gm:view({'Position2D', 'Size2D'}) do
            -- tester la collision entre mover et obstacle
        end
    end
end
```

---

## 6. Ce qui se passe pendant une frame

Pendant une frame, pendant que les systèmes tournent, les changements demandés par un script sont **appliqués à la fin de la frame**, tous ensemble :

- `gm:destroyEntity`, `gm:addComponent`, `gm:removeComponent` et `gm:destroySystem` sont appliqués à la fin de la frame ;
- `gm:createEntity` est immédiat ;
- `gm:stop()` arrête le jeu après cette frame.

C'est ce qui permet à un système de détruire des entités pendant que le moteur les parcourt. Ça a deux effets visibles, dans la même frame :

```lua
function process(gm, e, f)
    gm:destroyEntity(e)
    gm:hasEntity(e)                   -- encore true jusqu'à la fin de la frame
end
```

```lua
function process(gm, e, pos)
    gm:addComponent(e, 'Velocity2D')
    gm:hasComponent(e, 'Velocity2D')  -- encore false jusqu'à la fin de la frame
end
```

Dans `start`, hors de la frame, chaque changement est immédiat.

Un changement qui ne peut pas être appliqué à la fin de la frame arrête alors le jeu, par exemple un composant ajouté à une entité détruite dans la même frame (voir la [section 9](#9-les-erreurs-et-comment-les-régler)).

**Les systèmes ne se créent que dans `start`.** Appeler `gm:createSystem` ou `gm:addSystem` depuis un système arrête le jeu avec une erreur : le nouveau système tournerait dans la frame en cours, et un système créé pour chaque entité à chaque frame ferait s'accumuler les systèmes jusqu'à ralentir le jeu.

---

## 7. Un jeu complet

Un vaisseau qui traverse la fenêtre et rebondit sur ses bords. La mise en place (le `config.conf`, avec le module `Window`) est dans [le guide pour lancer un jeu](ENGINE_GAME.fr.md#5-un-jeu-complet).

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

À chaque frame : `quit` arrête le jeu si la fenêtre doit se fermer, `bounce` fait demi-tour au vaisseau sur les bords, `MasterVelocity2D` le déplace, `DrawSprite2D` le dessine.

---

## 8. À faire / à ne pas faire

### À faire

- **Définir `start` dans chaque script.**
- **Avoir un système qui appelle `gm:stop()`** quand `gm:shouldClose()` renvoie `true` ([arrêter le jeu](#arrêter-le-jeu)).
- **Créer les entités et les systèmes dans `start`**, et mettre ce qui tourne à chaque frame dans des systèmes.
- **Donner à un système la liste des composants sur lesquels il travaille**, plutôt que boucler avec une vue dans un système appelé une fois par frame : le moteur choisit alors les entités pour vous.
- **Préférer les systèmes `Master…` du moteur**, sauf si certaines entités ont besoin de leur propre horloge.
- **Utiliser `local`** pour les variables et les fonctions utilitaires du script.
- **Écrire les chemins par rapport au dossier depuis lequel le jeu est lancé.**

### À ne pas faire

- **Ne pas appeler `gm:createSystem` ou `gm:addSystem` depuis un système** ([section 6](#6-ce-qui-se-passe-pendant-une-frame)).
- **Ne pas compter sur un changement fait dans un système pour être visible dans la même frame** ([section 6](#6-ce-qui-se-passe-pendant-une-frame)).
- **Ne pas donner deux fois le même composant à une entité** : l'enlever d'abord, ou modifier ses champs avec `getComponent`.
- **Ne pas utiliser une entité comme clé de table** : utiliser `tostring(e)` ([garder des données sur une entité](#garder-des-données-sur-une-entité)).

---

## 9. Les erreurs et comment les régler

Chaque erreur arrête le jeu avec le code de sortie `84` et un message `Engine Error: Script error: ...`. Les lignes `stack traceback` sous le message donnent le fichier et la ligne où elle s'est produite.

| Message | Cause | Solution |
|---|---|---|
| `attempt to call a nil value` | un script n'a pas de fonction `start` | définir `function start(gm) ... end` |
| `scripts/a.lua:1: ')' expected near 'end'` | une erreur de syntaxe Lua, à ce fichier et cette ligne | corriger la syntaxe |
| `Unknown component: Postion2D` | le nom n'est pas un des [composants](#les-composants) | corriger le nom (la casse compte) |
| `Unknown system: DrawSprit2D` | le nom n'est pas un des [systèmes du moteur](#les-systèmes-du-moteur) | corriger le nom |
| `gm:keyboard needs the Keyboard module` | `gm:keyboard()`, `gm:mouse()`, ou une fonction de la fenêtre comme `gm:shouldClose()`, sans le module correspondant | ajouter `Keyboard`, `Mouse` ou `Window` aux `modules` de `config.conf` |
| `gm:setListenerPosition needs the Audio module` | `gm:setListenerPosition()` sans le module `Audio` | ajouter `Audio` aux `modules` de `config.conf` |
| `Unknown component: KeyboardManager` | un script essaie de créer un clavier ou une souris | utiliser `gm:keyboard()` ou `gm:mouse()` |
| `Sound needs the Audio module` | un `Sound` sans le module `Audio` | ajouter `Audio` aux `modules` de `config.conf` |
| `Cannot load sound sounds/shot.mp3: Resource does not exist` | le fichier du son n'existe pas à ce chemin | corriger le chemin, écrit par rapport au dossier depuis lequel le jeu est lancé |
| `sol: cannot write to a readonly property` | un script écrit `mouse.x`, `mouse.y` ou `mouse.wheel` | ne faire que les lire |
| `gm:createSystem cannot be called from a system, only from start` | `gm:createSystem` ou `gm:addSystem` appelé depuis un système | créer le système dans `start`, et l'arrêter ou le laisser sans effet avec une variable du script |
| `Camera can be created only once` | une deuxième `Camera3D` | garder une seule caméra et la déplacer avec sa `Position3D` |
| `Invalid arguments for Position2D: no field 'z'` | la table donne un champ que le composant n'a pas | enlever ou renommer le champ |
| `Invalid arguments for Position2D: field 'x' expects a number, got a string` | un champ du mauvais type | donner une valeur du bon type |
| `Invalid arguments for Texture2D: missing 'path'` | un composant construit à partir d'un fichier, sans `path` | donner `{path = '...'}` |
| `Invalid component list: element 2 is not a name` | la liste d'un système ou d'une vue contient autre chose qu'une chaîne | ne mettre que des noms dans la liste |
| `Invalid component list: Position2D is in it twice` | un nom est deux fois dans la liste | enlever le doublon |
| `Already an existing component linked to this entity` | l'entité a déjà ce composant | le modifier avec `getComponent`, ou l'enlever d'abord |
| `No such component.` | `getComponent` ou `removeComponent` sur un composant que l'entité n'a pas | vérifier d'abord avec `hasComponent` |
| `sol: no matching function call takes this number of arguments and the specified types` | de mauvais arguments à une fonction du moteur, par exemple `gm:createSystem('Position2D', f)` au lieu de `gm:createSystem({'Position2D'}, f)` | comparer les arguments avec l'[aide-mémoire](#11-aide-mémoire-de-lapi) |
| `sol: cannot set (new_index) into this object: no defined new_index operation on usertype` | un système écrit un champ que le composant n'a pas, par exemple `pos.z = 1` sur un `Position2D` | utiliser un champ du [tableau](#les-composants) |
| `attempt to index a nil value (global 'string')` | `string`, `table`, `os` et `io` ne sont pas disponibles | écrire le calcul avec la bibliothèque de base et `math` |
| `Invalid arguments for Clock: field 'elapsed' expects a function, got a number` | `elapsed` donné à `addComponent` | ne donner que `speed` : `elapsed` se lit avec `clock:elapsed()` |

### `Engine Error: No such entity.` ou `No such component.`, sans `Script error`

Elles arrivent **à la fin d'une frame**, sans fichier ni ligne : un changement demandé par un système n'a pas pu être appliqué une fois la frame terminée ([section 6](#6-ce-qui-se-passe-pendant-une-frame)). Par exemple :

- `gm:addComponent` ou `gm:removeComponent` sur une entité qu'un système a détruite dans la même frame ;
- `gm:removeComponent` deux fois sur le même composant dans la même frame.

**Solution :** vérifier `gm:hasEntity` / `gm:hasComponent` avant, ou faire en sorte qu'un seul système détruise ou enlève.

---

## 10. Pour les développeurs du moteur : exposer un composant ou un système

Tout ce qui rend le moteur utilisable depuis Lua est listé dans un seul fichier, [`Engine/Game/src/EngineBindings.cpp`](../Engine/Game/src/EngineBindings.cpp) : une ligne `bindComponent` par composant, et une ligne `makeSystem` par système.

```cpp
bindComponent<Position2D>(lua, "Position2D", "x", &Position2D::x, "y", &Position2D::y);
{"DrawSprite2D", makeSystem(Sprite2D::systemDrawSprite2D)},
```

Les cas particuliers (champs imbriqués, méthodes, enums, composants construits à partir d'un fichier), les modules et les nouvelles fonctions `gm:` sont expliqués dans [le guide pour étendre le moteur](ENGINE_EXTENDING.fr.md).

---

## 11. Aide-mémoire de l'API

| Appel | Effet |
|---|---|
| `gm:createEntity()` | crée une entité et la renvoie |
| `gm:destroyEntity(e)` | détruit une entité et ses composants |
| `gm:hasEntity(e)` | si l'entité existe |
| `gm:stop()` | arrête le jeu après la frame en cours |
| `gm:shouldClose()` | si le bouton de fermeture ou `Échap` a été pressé |
| `gm:fps()`, `gm:setFps(n)` | lit ou change les images par seconde de la fenêtre |
| `gm:fullScreen()`, `gm:setFullScreen(b)` | lit ou change si la fenêtre est en plein écran |
| `gm:hideCursor()`, `gm:setHideCursor(b)` | lit ou change si le curseur est caché et gardé dans la fenêtre |
| `gm:setListenerPosition(x, y)` | déplace le point qui entend les sons |
| `gm:addComponent(e, nom [, champs])` | ajoute un composant, construit à partir de la table |
| `gm:getComponent(e, nom)` | le vrai composant |
| `gm:hasComponent(e, nom)` | si l'entité a le composant |
| `gm:removeComponent(e, nom)` | enlève le composant |
| `gm:countComponents(nom)` | combien d'entités ont le composant |
| `gm:createSystem(func)` | appelle `func(gm)` à chaque frame |
| `gm:createSystem({noms}, func)` | appelle `func(gm, e, composants...)` à chaque frame, pour chaque entité qui les a |
| `gm:addSystem(nom)` | ajoute un des systèmes du moteur |
| `gm:destroySystem(id)` | arrête un système, avec l'id que renvoie `createSystem` ou `addSystem` |
| `gm:hasSystem(id)` | si le système existe |
| `gm:keyboard()` | le clavier : `isPressed`, `isDown`, `isReleased` avec une valeur de `Key` |
| `gm:mouse()` | la souris : `isPressed`, `isDown`, `isReleased` avec une valeur de `MouseButton`, et `x`, `y`, `wheel` |
| `gm:elapsed()` | les millisecondes de la frame précédente, selon l'horloge du jeu |
| `for e, composants... in gm:view({noms}) do` | parcourt les entités qui ont les composants |
