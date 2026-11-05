# Écrire un jeu en Lua

> 🌐 **Langue :** Français · [English](LUA_GUIDE.en.md)

Avec `engine-game`, le contenu d'un jeu s'écrit en Lua : les scripts créent les entités, leur donnent les composants du moteur, et enregistrent les systèmes qui tournent à chaque frame. La partie C++ du jeu ne contient que ses réglages (`RType::GameConfig`).

- **Public visé :** toute personne qui écrit les scripts d'un jeu, et, dans la [section 10](#10-pour-les-développeurs-du-moteur--exposer-un-composant-ou-un-système), les développeurs du moteur qui rendent un nouveau composant ou système utilisable depuis Lua.
- **Pour mettre en place le jeu** (`GameConfig`, modules, dossier, CMake), consultez [le guide d'`engine-game`](ENGINE_GAME.fr.md).

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

Le moteur charge tous les fichiers `.lua` du dossier `TestLua/` et de ses sous-dossiers, dans l'ordre alphabétique de leur chemin, puis appelle la fonction `start` de chacun, **une seule fois**, avec le `GameManager` :

```lua
function start(gm)
    print('le jeu démarre')
end
```

- **Chaque script doit définir `start`.** Un script qui n'en a pas arrête le jeu (voir la [section 9](#9-les-erreurs-et-comment-les-régler)).
- **`start` crée les choses ; les systèmes les font tourner.** Ce qui doit se passer à chaque frame va dans un système, enregistré depuis `start` ([section 4](#4-les-systèmes)).
- **Chaque script a ses propres globales.** Une variable ou une fonction globale définie dans `a.lua` n'est pas visible depuis `b.lua` : deux scripts peuvent donc tous les deux appeler leur fonction `process` sans conflit. Utilisez quand même `local` pour tout le reste.
- **La bibliothèque de base de Lua est disponible** : `print`, `pairs`, `ipairs`, `pcall`, `error`, `type`, `tostring`, `tonumber`…
- **Les chemins** (images, polices, modèles) sont relatifs au dossier depuis lequel le jeu est lancé, celui qui contient `TestLua/`.

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
| `Clock` | `speed` (1) | l'horloge propre à une entité |
| `Texture2D` | `path` (**obligatoire**) | une image, chargée depuis `path` |
| `Sprite2D` | `scale` (1), `rotation` (0), `flip` (false) | dessine la `Texture2D` de l'entité |
| `SpriteSheet2D` | `rows` (1), `columns` (1), `currentTile` (0), `frameDuration` (100, en ms), `animation` (true), `scale` (1), `rotation` (0), `flip` (false) | dessine et anime une case de la `Texture2D` à la fois |
| `Font` | `path` (**obligatoire**) | une police, chargée depuis `path` |
| `Text2D` | `str` ("Hello World"), `fontSize` (10), `rotation` (0), `spacing` (0) | dessine un texte avec la `Font` de l'entité |
| `Position3D` | `x` (0), `y` (0), `z` (0) | position dans l'espace 3D |
| `Model3D` | `path` (**obligatoire**) | un modèle 3D, chargé depuis `path` |
| `ModelAnimation3D` | `path` (**obligatoire**) | les animations d'un modèle 3D |
| `Sprite3D` | `scale` (1) | dessine le `Model3D` de l'entité |
| `SpriteSheet3D` | `index` (0), `frameDuration` (100, en ms), `animation` (true), `scale` (1) | joue une animation du `Model3D` |
| `Listener` | | le point qui entend les sons 2D |

Les quatre composants marqués **obligatoire** sont construits à partir d'un fichier : `gm:addComponent(e, 'Texture2D', {path = 'TestLua/sprites/ship.png'})`.

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
| `Listener2D` | `Listener`, `Position2D` | déplace le point qui entend les sons |
| `KeyboardManagerClear` | le clavier | vide les touches lues pendant la frame |

Les systèmes `Master…` utilisent l'horloge du jeu, que le moteur fait toujours tourner : c'est le choix le plus simple. Les autres utilisent une `Clock` donnée à chaque entité, pour ralentir ou accélérer certaines entités seulement (avec son `speed`), et ont aussi besoin du système `Clock`.

Les systèmes de la fenêtre, du clavier, de la souris et de l'horloge du jeu sont enregistrés par les [modules](ENGINE_GAME.fr.md#3-les-modules), une seule fois : ils ne sont pas dans ce tableau, pour qu'aucun script ne les fasse tourner deux fois par frame. Les autres systèmes du moteur (`Draw3D`, `UpdateCamera`, `UpdateKeybind`, `Sound`, `Sound2D`) agissent sur des composants créés depuis le C++.

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
- `gm:createEntity` est immédiat.

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

Un vaisseau qui traverse la fenêtre et rebondit sur ses bords. La mise en place (`Setup.cpp` avec les modules `Window` et `Keyboard`) est dans [le guide d'`engine-game`](ENGINE_GAME.fr.md#5-un-jeu-complet).

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

À chaque frame : `bounce` fait demi-tour au vaisseau sur les bords, `MasterVelocity2D` le déplace, `DrawSprite2D` le dessine.

---

## 8. À faire / à ne pas faire

### À faire

- **Définir `start` dans chaque script.**
- **Créer les entités et les systèmes dans `start`**, et mettre ce qui tourne à chaque frame dans des systèmes.
- **Donner à un système la liste des composants sur lesquels il travaille**, plutôt que boucler avec une vue dans un système appelé une fois par frame : le moteur choisit alors les entités pour vous.
- **Préférer les systèmes `Master…` du moteur**, sauf si certaines entités ont besoin de leur propre horloge.
- **Utiliser `local`** pour les variables et les fonctions utilitaires du script.
- **Écrire les chemins par rapport au dossier qui contient `TestLua/`.**

### À ne pas faire

- **Ne pas appeler `gm:createSystem` ou `gm:addSystem` depuis un système** ([section 6](#6-ce-qui-se-passe-pendant-une-frame)).
- **Ne pas compter sur un changement fait dans un système pour être visible dans la même frame** ([section 6](#6-ce-qui-se-passe-pendant-une-frame)).
- **Ne pas donner deux fois le même composant à une entité** : l'enlever d'abord, ou modifier ses champs avec `getComponent`.

---

## 9. Les erreurs et comment les régler

Chaque erreur arrête le jeu avec le code de sortie `84` et un message `Engine Error: Script error: ...`. Les lignes `stack traceback` sous le message donnent le fichier et la ligne où elle s'est produite.

| Message | Cause | Solution |
|---|---|---|
| `attempt to call a nil value` | un script n'a pas de fonction `start` | définir `function start(gm) ... end` |
| `TestLua/a.lua:1: ')' expected near 'end'` | une erreur de syntaxe Lua, à ce fichier et cette ligne | corriger la syntaxe |
| `Unknown component: Postion2D` | le nom n'est pas un des [composants](#les-composants) | corriger le nom (la casse compte) |
| `Unknown system: DrawSprit2D` | le nom n'est pas un des [systèmes du moteur](#les-systèmes-du-moteur) | corriger le nom |
| `gm:createSystem cannot be called from a system, only from start` | `gm:createSystem` ou `gm:addSystem` appelé depuis un système | créer le système dans `start`, et l'arrêter ou le laisser sans effet avec une variable du script |
| `Invalid arguments for Position2D: no field 'z'` | la table donne un champ que le composant n'a pas | enlever ou renommer le champ |
| `Invalid arguments for Position2D: field 'x' expects a number, got a string` | un champ du mauvais type | donner une valeur du bon type |
| `Invalid arguments for Texture2D: missing 'path'` | un composant construit à partir d'un fichier, sans `path` | donner `{path = '...'}` |
| `Invalid component list: element 2 is not a name` | la liste d'un système ou d'une vue contient autre chose qu'une chaîne | ne mettre que des noms dans la liste |
| `Invalid component list: Position2D is in it twice` | un nom est deux fois dans la liste | enlever le doublon |
| `Already an existing component linked to this entity` | l'entité a déjà ce composant | le modifier avec `getComponent`, ou l'enlever d'abord |
| `No such component.` | `getComponent` ou `removeComponent` sur un composant que l'entité n'a pas | vérifier d'abord avec `hasComponent` |
| `sol: no matching function call takes this number of arguments and the specified types` | de mauvais arguments à une fonction du moteur, par exemple `gm:createSystem('Position2D', f)` au lieu de `gm:createSystem({'Position2D'}, f)` | comparer les arguments avec l'[aide-mémoire](#11-aide-mémoire-de-lapi) |
| `sol: cannot set (new_index) into this object: no defined new_index operation on usertype` | un système écrit un champ que le composant n'a pas, par exemple `pos.z = 1` sur un `Position2D` | utiliser un champ du [tableau](#les-composants) |
| `attempt to index a nil value (global 'math')` | `math`, `string`, `table`, `os` et `io` ne sont pas disponibles | écrire le calcul avec la bibliothèque de base |

### `Engine Error: No such entity.` ou `No such component.`, sans `Script error`

Elles arrivent **à la fin d'une frame**, sans fichier ni ligne : un changement demandé par un système n'a pas pu être appliqué une fois la frame terminée ([section 6](#6-ce-qui-se-passe-pendant-une-frame)). Par exemple :

- `gm:addComponent` ou `gm:removeComponent` sur une entité qu'un système a détruite dans la même frame ;
- `gm:removeComponent` deux fois sur le même composant dans la même frame.

**Solution :** vérifier `gm:hasEntity` / `gm:hasComponent` avant, ou faire en sorte qu'un seul système détruise ou enlève.

---

## 10. Pour les développeurs du moteur : exposer un composant ou un système

Tout ce qui rend le moteur utilisable depuis Lua est listé dans un seul fichier, [`Engine/Game/src/EngineBindings.cpp`](../Engine/Game/src/EngineBindings.cpp).

### Un composant

Ajoutez une ligne `bindComponent` dans `Game::bindEngineComponents` : le type C++, le nom utilisé par les scripts, puis chaque champ sous la forme `"nom", &Type::membre`.

```cpp
bindComponent<Position2D>(lua, "Position2D",
    "x", &Position2D::x, "y", &Position2D::y);
```

Cette seule ligne donne au composant tout ce qui est dans la [section 3](#3-les-composants) et dans les systèmes et les vues : `addComponent` avec une table et des valeurs par défaut, la vérification des champs et ses messages, `getComponent`, `hasComponent`, `removeComponent`, `countComponents`, et son utilisation dans `createSystem` et `gm:view`. Changer un champ ne demande de modifier que cette ligne.

- **Un composant sans constructeur par défaut**, comme `Texture2D`, est construit à partir du champ `path` du script. Ne lui donnez aucun champ : `bindComponent<Texture2D>(lua, "Texture2D");`.
- **Un champ d'un champ** s'expose avec `nested`. `SpriteSheet2D::recNb.height` devient `rows` :

  ```cpp
  "rows", nested(&SpriteSheet2D::recNb, &Size2D::height),
  ```

- **Un membre `std::chrono::milliseconds`** s'expose comme un nombre de millisecondes avec `milliseconds` :

  ```cpp
  "frameDuration", milliseconds(&SpriteSheet2D::frameDuration),
  ```

### Un système

Ajoutez une ligne dans `Game::systemFactories` : le nom utilisé par les scripts, qui est celui de la fonction C++ sans son préfixe `system`, puis `makeSystem` avec la fonction.

```cpp
{"DrawSprite2D", makeSystem(Sprite2D::systemDrawSprite2D)},
```

`makeSystem` lit les composants dans les paramètres de la fonction elle-même, avec ou sans `GameManager &` en premier : il n'y a rien d'autre à écrire.

---

## 11. Aide-mémoire de l'API

| Appel | Effet |
|---|---|
| `gm:createEntity()` | crée une entité et la renvoie |
| `gm:destroyEntity(e)` | détruit une entité et ses composants |
| `gm:hasEntity(e)` | si l'entité existe |
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
| `for e, composants... in gm:view({noms}) do` | parcourt les entités qui ont les composants |
