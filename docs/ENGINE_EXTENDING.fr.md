# Étendre le moteur : composants, systèmes, modules

> 🌐 **Langue :** Français · [English](ENGINE_EXTENDING.en.md)

Ce guide explique comment ajouter au moteur un composant, un système ou un module, et comment les rendre utilisables depuis les scripts Lua. Chaque ajout se fait en C++, à un endroit précis, et le reste du moteur (vérification des champs, messages d'erreur, systèmes et vues Lua) en profite sans autre code.

- **Public visé :** les développeurs du moteur.
- **Pour écrire un composant ou un système en C++ dans un jeu**, consultez [le guide de l'ECS](ECS_GUIDE.fr.md). **Pour les utiliser depuis Lua**, consultez [le guide Lua](LUA_GUIDE.fr.md).

---

## Sommaire

1. [Où se trouve le code](#1-où-se-trouve-le-code)
2. [Ajouter un composant](#2-ajouter-un-composant)
3. [Ajouter un système](#3-ajouter-un-système)
4. [Ajouter un module](#4-ajouter-un-module)
5. [Ajouter une fonction Lua au `GameManager`](#5-ajouter-une-fonction-lua-au-gamemanager)
6. [Récapitulatif](#6-récapitulatif)

---

## 1. Où se trouve le code

| Bibliothèque | Dossier | Contenu |
|---|---|---|
| `engine-system` | [`Engine/System`](../Engine/System) | l'ECS, et les composants de base (`Position2D`, `Velocity2D`, `Clock`…) dans `include/BuiltIn` |
| `engine-graphical2D` | [`Engine/Graphical2D`](../Engine/Graphical2D) | la fenêtre, le clavier, la souris, les sprites et les textes 2D |
| `engine-graphical3D` | [`Engine/Graphical3D`](../Engine/Graphical3D) | les modèles et la caméra 3D |
| `engine-audio` | [`Engine/Audio`](../Engine/Audio) | les sons et le point qui les entend |
| `engine-game` | [`Engine/Game`](../Engine/Game) | le `main`, les modules et tout le lien avec Lua |

Dans `engine-game`, tout est dans le namespace `RType::Game`, une classe par rôle :

| Fichier | Rôle |
|---|---|
| `Runner` | le `main` du moteur : modules, scripts, boucle |
| `Modules` | les modules que le jeu peut demander |
| `Scripts` | le chargement des scripts et la conversion de leurs erreurs |
| `GameManagerBinding` | expose le `GameManager` et les entités, puis appelle les autres bindings |
| `ComponentBinding` | le registre des composants utilisables depuis Lua |
| `SystemBinding` | `gm:createSystem`, `gm:addSystem`, `gm:destroySystem`, `gm:hasSystem` |
| `ViewBinding` | `gm:view` |
| `InputBinding` | `gm:keyboard`, `gm:mouse`, les tables `Key` et `MouseButton` |
| `TimeBinding` | `gm:elapsed` |
| **`EngineBindings.cpp`** | **les listes des composants et des systèmes du moteur utilisables depuis Lua : c'est le fichier à modifier pour en ajouter un** |

---

## 2. Ajouter un composant

### Écrire le composant

Un composant est une struct de données, placée dans le dossier `include` de la bibliothèque qui la concerne. Donnez une valeur par défaut à chaque champ : c'est elle qu'un script obtient quand il ne donne pas le champ.

```cpp
// Engine/System/include/BuiltIn/Health.hpp
namespace RType {
    struct Health {
        int hp = 100;
        int max = 100;
    };
}
```

S'il a un `.cpp` (pour ses systèmes, par exemple), ajoutez-le à la liste `SRCS_...` de sa bibliothèque dans [`Engine/CMakeLists.txt`](../Engine/CMakeLists.txt). Les règles d'un bon composant (déplaçable, sans référence à sa propre entité…) sont dans [le guide de l'ECS](ECS_GUIDE.fr.md#3-les-composants).

### Le rendre utilisable depuis Lua

Ajoutez une ligne `bindComponent` dans `ComponentBinding::bindEngineComponents`, dans [`EngineBindings.cpp`](../Engine/Game/src/EngineBindings.cpp) : le type C++, le nom utilisé par les scripts, puis chaque champ sous la forme `"nom", &Type::membre`.

```cpp
bindComponent<Health>(lua, "Health", "hp", &Health::hp, "max", &Health::max);
```

Cette seule ligne donne au composant, côté Lua :

- `gm:addComponent(e, 'Health', {hp = 50})`, avec les valeurs par défaut pour les champs omis ;
- la vérification des champs, avec ses messages : `no field 'hpp'`, `field 'hp' expects a number, got a string` ;
- `gm:getComponent`, `gm:hasComponent`, `gm:removeComponent`, `gm:countComponents` ;
- son utilisation dans la liste d'un `gm:createSystem` et d'un `gm:view`.

Changer un champ ne demande de modifier que cette ligne.

### Les champs qui ne sont pas de simples membres

| Cas | Comment l'écrire | Exemple |
|---|---|---|
| un champ d'un champ | `nested` | `"rows", nested(&SpriteSheet2D::recNb, &Size2D::height)` |
| un `std::chrono::milliseconds` | `milliseconds`, qui l'expose en nombre de millisecondes | `"frameDuration", milliseconds(&SpriteSheet2D::frameDuration)` |
| une méthode | une lambda qui prend le composant en premier | `"elapsed", [](const Clock &c) { return c.getElapsed().count(); }` |
| une méthode du composant lui-même | un pointeur de méthode | `"setVolume", &Sound::setVolume` |
| un membre d'une classe parente | `sol::property`, avec une lambda pour lire et une pour écrire | `"loop", sol::property([](const Sound &s) { return s.loop; }, [](Sound &s, bool v) { s.loop = v; })` |
| un enum | déclarer ses valeurs avec `lua.new_enum`, en haut de `bindEngineComponents` ; le champ s'écrit ensuite normalement | `lua.new_enum<SoundInfo::State>("SoundState", {{"PLAY", SoundInfo::State::PLAY}, ...})` |

Une méthode ne peut pas être donnée à `addComponent` : `{elapsed = 5}` est refusé avec `field 'elapsed' expects a function, got a number`.

### Un composant construit à partir d'un fichier

Un composant **sans constructeur par défaut**, comme `Texture2D`, est construit avec le champ `path` du script, puis reçoit les autres champs de la table. Sa ligne ne change pas :

```cpp
bindComponent<Texture2D>(lua, "Texture2D");
```

Si son constructeur demande **plus que le chemin**, spécialisez `ComponentBinding::construct` dans `EngineBindings.cpp`, avant `bindEngineComponents`. Elle reçoit le `GameManager`, pour aller chercher ce qui manque. C'est ce que fait `Sound`, qui a besoin du point qui entend les sons, créé par le module `Audio` :

```cpp
template <>
Sound ComponentBinding::construct<Sound>(GameManager &gm, const std::string &path)
{
    return Sound(GameManagerBinding::findModuleComponent<Listener>(gm, "Sound", "Audio"), path);
}
```

### Le documenter

Ajoutez une ligne au tableau [des composants du guide Lua](LUA_GUIDE.fr.md#les-composants), en français et en anglais : son nom, ses champs avec leur valeur par défaut, et son rôle.

---

## 3. Ajouter un système

### Écrire le système

Un système du moteur est une fonction statique de la classe du composant principal, nommée `system` suivi de ce qu'elle fait. Ses paramètres sont les composants sur lesquels elle travaille, `const` pour ceux qu'elle ne fait que lire, avec éventuellement le `GameManager` en premier :

```cpp
struct Health {
    int hp = 100;
    int max = 100;

    static void systemRegenerate(Health &health, const Clock &clock);
};
```

Le moteur l'appelle à chaque frame, une fois pour chaque entité qui a tous ces composants.

### Le rendre utilisable depuis Lua

Ajoutez une ligne dans `SystemBinding::engineSystems`, dans [`EngineBindings.cpp`](../Engine/Game/src/EngineBindings.cpp) : le nom utilisé par les scripts, qui est celui de la fonction sans son préfixe `system`, puis `makeSystem` avec la fonction.

```cpp
{"Regenerate", makeSystem(Health::systemRegenerate)},
```

`makeSystem` lit les composants dans les paramètres de la fonction elle-même : il n'y a rien d'autre à écrire. Un script l'ajoute ensuite avec `gm:addSystem('Regenerate')`.

Un système qui appartient à un module (voir la [section 4](#4-ajouter-un-module)) **n'est pas** dans cette liste : le module l'enregistre une fois, et un script ne doit pas pouvoir le faire tourner une deuxième fois.

### Le documenter

Ajoutez une ligne au tableau [des systèmes du moteur du guide Lua](LUA_GUIDE.fr.md#les-systèmes-du-moteur), en français et en anglais : son nom, les composants sur lesquels il agit, et son rôle.

---

## 4. Ajouter un module

Un module est une fonctionnalité que le jeu active par son nom, dans `GameConfig.modules`. Il crée, une seule fois et avant les scripts, une entité avec le composant qu'il possède, et le système qui le met à jour. Ce composant appartient au moteur : un script le lit, mais ne peut ni en créer un autre, ni ajouter le système une deuxième fois.

### Écrire le module

Ajoutez une entrée à `Modules::allowed`, dans [`Modules.cpp`](../Engine/Game/src/Modules.cpp) : son nom, les modules dont il dépend, et ce qu'il crée.

```cpp
{"Audio",
    {{},
        [](GameManager &gm, const GameInfo &) {
            gm.addComponent<Listener>(gm.createEntity());
            gm.createSystem<Sound>(Sound::systemSound);
        }}},
```

- **Les dépendances** sont vérifiées avant que quoi que ce soit ne tourne : `{"Window"}` pour un module qui a besoin de la fenêtre, comme `Keyboard`.
- **Son composant n'a pas de `bindComponent`**, et **ses systèmes ne sont pas dans `engineSystems`**. Un script qui essaie d'en créer obtient `Unknown component` ou `Unknown system`.
- Le lambda reçoit `GameConfig`, pour un module qui a des réglages, comme la taille de la fenêtre pour `Window`.

### Donner aux scripts l'accès à son composant

Un script n'accède au composant d'un module que par une fonction du `GameManager`, comme `gm:keyboard()` ou `gm:elapsed()`. Elles sont regroupées dans une classe de binding, sur le modèle d'[`InputBinding`](../Engine/Game/include/InputBinding.hpp) et de [`TimeBinding`](../Engine/Game/include/TimeBinding.hpp) :

1. **Un en-tête** dans `Engine/Game/include`, avec une classe du namespace `RType::Game` qui n'a qu'une fonction statique publique `registerFunctions(sol::state &lua)`.
2. **Un `.cpp`** dans `Engine/Game/src`, qui l'implémente :

   ```cpp
   void InputBinding::registerFunctions(sol::state &lua)
   {
       lua.new_usertype<KeyboardManager>("KeyboardManager", sol::no_constructor,
           "isDown", contains(&KeyboardManager::keyDown));
       lua["GameManager"]["keyboard"] = [](GameManager &gm) -> KeyboardManager & {
           return GameManagerBinding::findModuleComponent<KeyboardManager>(
               gm, "gm:keyboard", "Keyboard");
       };
   }
   ```

   - **`sol::no_constructor`** empêche un script d'écrire `KeyboardManager.new()`.
   - **`GameManagerBinding::findModuleComponent`** trouve le composant créé par le module. Sans le module, il lève `ModuleNotLoaded`, que le script reçoit sous la forme `gm:keyboard needs the Keyboard module`.
   - **Renvoyer une référence** (`-> KeyboardManager &`) donne au script le vrai composant, mis à jour à chaque frame.
   - Si le script n'a besoin que d'une valeur, renvoyez-la directement, sans exposer le composant : `gm:elapsed()` renvoie le temps de la `MasterClock` sans exposer la `MasterClock`.
3. **L'appel** à `registerFunctions` dans `GameManagerBinding::registerFunctions`.
4. **Le `.cpp`** dans la liste `SRCS_GAME` d'[`Engine/CMakeLists.txt`](../Engine/CMakeLists.txt).

### Le documenter

- Une ligne au tableau [des modules du guide d'`engine-game`](ENGINE_GAME.fr.md#3-les-modules), en français et en anglais : son nom, ce qu'il crée, comment les scripts y accèdent, et ses dépendances.
- Une sous-section dans [le guide Lua](LUA_GUIDE.fr.md), avec un exemple, et ses fonctions dans l'[aide-mémoire](LUA_GUIDE.fr.md#11-aide-mémoire-de-lapi).

---

## 5. Ajouter une fonction Lua au `GameManager`

Toute fonction appelable par `gm:nom(...)` s'ajoute dans le `registerFunctions` d'une classe de binding, avec une lambda qui prend le `GameManager` en premier :

```cpp
lua["GameManager"]["countComponents"] =
    [](GameManager &gm, const std::string &name) {
        return getEntry(name).count(gm);
    };
```

- **Les erreurs** se signalent en levant une exception qui hérite de `EngineException`, déclarée dans [`Exceptions.hpp`](../Engine/System/include/Exceptions.hpp). Le script la reçoit comme une erreur Lua, avec son message, et le jeu s'arrête avec ce message s'il ne l'attrape pas avec `pcall`.
- **Les arguments de mauvais type** sont refusés par sol, sans code à écrire : `stack index 2, expected number, received string`.
- **Une fonction qui crée un système** se place dans `SystemBinding` et appelle `checkNotUpdating` en premier, pour refuser d'être appelée depuis un système.

---

## 6. Récapitulatif

| Ajouter | Code | Lua | Doc |
|---|---|---|---|
| un composant | la struct, et son `.cpp` dans `SRCS_...` | une ligne `bindComponent` dans `EngineBindings.cpp` | le tableau des composants du guide Lua |
| un système | une fonction statique `systemNom` | une ligne `makeSystem` dans `engineSystems`, sauf s'il appartient à un module | le tableau des systèmes du moteur du guide Lua |
| un module | une entrée dans `Modules::allowed` | une classe de binding avec `findModuleComponent`, appelée par `GameManagerBinding` | le tableau des modules du guide d'`engine-game`, et une sous-section du guide Lua |
| une fonction `gm:` | | une lambda dans le `registerFunctions` d'une classe de binding | l'aide-mémoire du guide Lua |
