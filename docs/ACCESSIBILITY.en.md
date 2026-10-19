# Accessibility

> 🌐 **Language:** [Français](ACCESSIBILITY.fr.md) · English

This document presents what the engine offers for **accessibility**. For each feature, it explains what it allows, why it is useful, who it is for, and how to use it.

- **Audience:** anyone working on the engine or on a game who wants as many players as possible to be able to play.
- **Prerequisite:** the [game making guide](GAME_GUIDE.en.md) for how to use the components. The code examples follow its conventions.

---

## Table of contents

1. [Why it matters](#1-why-it-matters)
2. [The guidelines used](#2-the-guidelines-used)
3. [The engine's features](#3-the-engines-features)
4. [Summary](#4-summary)
5. [Accessibility of the documentation](#5-accessibility-of-the-documentation)
6. [Sources](#6-sources)

---

## 1. Why it matters

A shoot'em up like R-Type asks a lot: reflexes, precision, repeated presses, a good reading of the screen. Each of these demands can hinder some players:

- **Motor disabilities** (reduced dexterity, tremors, chronic pain, use of a single hand): some keys are hard to reach, to mash or to hold.
- **Visual disabilities** (low vision, color blindness): small text or a low-contrast element can go unnoticed. According to Colour Blind Awareness, color blindness affects about **1 in 12 men (8%) and 1 in 200 women**, most often for red and green.
- **Cognitive disabilities** (difficulties with attention, reading): hard-to-read text or a busy screen quickly becomes tiring.
- **Temporary or hardware situations**: an arm in a cast, a foreign keyboard, a low-end machine.

Making a game accessible is mostly about **giving players choices**. The features below give many of them, and they benefit every other player too.

---

## 2. The guidelines used

This document relies on two recognized sets of guidelines in video games:

- **Game Accessibility Guidelines** (GAG): a list of recommendations sorted into three levels. The **basic** level is simple to implement and useful to many players. The **intermediate** level takes more work. The **advanced** level addresses more specific needs ([gameaccessibilityguidelines.com](https://gameaccessibilityguidelines.com/full-list/)).
- **Xbox Accessibility Guidelines** (XAG): Microsoft's recommendations, with more figures. This document uses XAG 101 (text display), XAG 102 (contrast) and XAG 107 (input) ([Microsoft Learn](https://learn.microsoft.com/en-us/gaming/accessibility/guidelines)).

For each feature, this document quotes the recommendation it meets, with its original title and level.

---

## 3. The engine's features

### 3.1 Rebinding keys and buttons (`Keybind`)

**What it is.** The `Keybind` component (`Engine/Graphical/include/Keybind.hpp`) rewrites keyboard and mouse input **before** the game reads it. It handles four kinds of mappings:

| Mapping | Member | Example |
|---|---|---|
| Key to key | `_keyToKey` | The up arrow acts as Z |
| Mouse button to mouse button | `_mouseButtonToMouseButton` | Swap left and right click |
| Key to mouse button | `_keyToMouseButton` | Space acts as a left click |
| Mouse button to key | `_mouseButtonToKey` | Left click acts as Space |

**Why it is good.**

- **Everyone picks their keys**: players put the controls where they can reach them comfortably.
- **Playing with a single hand** becomes possible: group everything on one side of the keyboard, or put shooting on the mouse and movement on the keyboard.
- **Keys can be added without removing any**: a key absent from the `Keybind` keeps its role. Making the up arrow act as Z leaves Z working, and the player has both.
- **Several keys can lead to the same action**: the up arrow and numpad 8 to Z, for instance.
- **Two keys can be swapped**: Z to up and up to Z at the same time. Mappings are computed from the keys actually pressed, so the swap works.
- **The game code doesn't change**: the game reads `keyDown` and `keyPressed` as usual, it simply receives keys that are already remapped. A settings screen only has to fill the `Keybind`.

**Who it is for.** Players who can only use one hand (permanent disability or temporary injury), who have reduced dexterity or range of motion, left-handed players, and anyone who prefers other keys.

**Recommendations.** GAG basic *"Allow controls to be remapped / reconfigured"*. XAG 107: *"Players should be given the option to remap all of the controls within the game itself"*.

**How to use it.** The `Keybind` goes on the entity that holds the `KeyboardManager` and the `MouseManager`. Its system is registered **right after** theirs, and **before** the game's systems:

```cpp
auto input = gm.createEntity();
gm.addComponent<RType::KeyboardManager>(input);
gm.addComponent<RType::MouseManager>(input);
gm.addComponent<RType::Keybind>(input);

RType::Keybind &bind = gm.getComponent<RType::Keybind>(input);
// The arrows on top of ZQSD
bind._keyToKey[RType::Key::Value::Up] = RType::Key::Value::Z;
bind._keyToKey[RType::Key::Value::Left] = RType::Key::Value::Q;
bind._keyToKey[RType::Key::Value::Down] = RType::Key::Value::S;
bind._keyToKey[RType::Key::Value::Right] = RType::Key::Value::D;
// Shoot with the mouse
bind._mouseButtonToKey[RType::MouseButton::Value::Left] = RType::Key::Value::Space;

gm.createSystem<RType::KeyboardManager>(RType::KeyboardManager::systemKeyboardManager);
gm.createSystem<RType::MouseManager>(RType::MouseManager::systemMouseManager);
gm.createSystem<RType::Keybind, RType::MouseManager, RType::KeyboardManager>(
    RType::Keybind::systemUpdateKeybind);
// ... then the game's systems
```

### 3.2 AZERTY and QWERTY layouts (`Key::Layout`)

**What it is.** The `KeyboardManager` takes a keyboard layout: `Key::Layout::AZERTY` (default) or `Key::Layout::QWERTY`. A `Key::Value` designates **the key that bears this letter** on a keyboard of the chosen layout: with AZERTY, `Key::Value::Z` is the key labeled Z on a French keyboard, with QWERTY, the one labeled Z on an English keyboard.

```cpp
gm.addComponent<RType::KeyboardManager>(input, RType::Key::Layout::QWERTY);
```

**Why it is good.**

- **Key names match the player's keyboard**: "Press Z" designates the key where Z is written.
- **Default keys suited to each keyboard**: combined with the `Keybind`, the game can offer ZQSD on an AZERTY keyboard and WASD on a QWERTY keyboard, two groups of keys at the same place under the hand.
- **The layout can change during the game**: `layout` is a public member, read every frame. An options menu can change it without restarting the game.

**Who it is for.** Players whose keyboard isn't French, which is most players in the world, and French speakers who use a QWERTY keyboard.

### 3.3 Pressed, held, released

**What it is.** The `KeyboardManager` (and the `MouseManager`, for buttons) fills three lists every frame:

| List | Content |
|---|---|
| `keyPressed` | Keys pressed **during this frame** |
| `keyDown` | Keys **held** |
| `keyReleased` | Keys released **during this frame** |

**Why it is good.** With these three lists, the same action can be triggered in several ways, depending on what the player can do:

- **Auto-fire**: shoot as long as the key is held (`keyDown`), with a delay between two shots, instead of one shot per press.
- **Toggle**: one press (`keyPressed`) starts a continuous action, a second press stops it. The player has nothing left to hold.
- **Action on release**: trigger an action when the key is released (`keyReleased`), which leaves time to cancel an accidental press. XAG 107 recommends it for menus.

**Who it is for.** Players who struggle to mash a key or to hold it for long: reduced dexterity, chronic pain, fatigue, tremors.

**Recommendations.** GAG intermediate *"Avoid / provide alternatives to requiring buttons to be held down"*. XAG 107: avoid fast repeated presses and offer *"Toggles and "auto" holds"*.

### 3.4 The mouse (`MouseManager`)

**What it is.** Mouse buttons (left, right, middle and four side buttons), with the same pressed, held and released lists as the keyboard, plus the position (`getPos()`) and the wheel (`getWheel()`).

**Why it is good.** The mouse is a **second device** to play with. With the `Keybind`, any keyboard action can move to a mouse button, including the side buttons of a gaming mouse. Players spread the controls across both hands, or group them on a single one.

**Who it is for.** Players more comfortable with a mouse than a keyboard, those who play with one hand, and those who use an adapted mouse (trackball, large-button mouse).

**Recommendation.** GAG intermediate *"Support more than one input device"*.

### 3.5 Text (`Text2D` and `Font`)

**What it is.** `Text2D` displays text with a **size** (`fontSize`), a **color** (`color`), **letter spacing** (`spacing`) and a rotation, in a **font** loaded from a file (`Font`).

**Why it is good.**

- **Adjustable size**: `fontSize` can change at any time, for instance from a "text size" setting.
- **Any font**: any TTF or OTF font, for instance a sans-serif font or a font designed for dyslexic readers.
- **Free color**: a "high contrast" mode can switch text to pure white or bright yellow.
- **Adjustable spacing** between letters, which makes reading easier.

**Who it is for.** Players with low vision or dyslexia, and those who play far from the screen or on a small screen.

**Recommended values** (XAG 101 and 102), which these settings make it possible to reach:

| Setting | Recommendation |
|---|---|
| Minimum default size, on PC | **18 px** high at 1920×1080 (36 px in 4K) |
| Scaling | Up to **200%** of the minimum size |
| Letter spacing | At least **0.12 times** the text size |
| Contrast of normal text | At least **4.5:1** with the background |
| Contrast of large text (36 px and above at 1080p, on PC) | At least **3:1** |
| High contrast mode | At least **7:1** |

With raylib, `fontSize` roughly matches the text height in pixels.

**Recommendations.** GAG basic *"Use an easily readable default font size"* and *"Provide high contrast between text/UI and background"*, GAG intermediate *"Provide a choice of text colour, low/high contrast choice as a minimum"*, GAG advanced *"Allow the font size to be adjusted"*.

### 3.6 Image tint and size

**What it is.** `Sprite2D` and `SpriteSheet2D` have a **tint** (`color`, white by default, meaning the original image) and a **scale** (`scale`).

**Why it is good.**

- **Darken the scenery**: tinting the background dark gray (for instance `raylib::Color{80, 80, 80, 255}`) makes the ship, enemies and shots stand out. XAG 102 cites as an example a game that lets players dim the scenery down to black.
- **Let players choose the colors** of important elements, such as enemy shots, so that they pick the one they distinguish best.
- **Enlarge game elements** with `scale`, so they stay visible.
- **Put a background behind text**: a one-pixel white image, enlarged with `scale` and tinted semi-transparent black, keeps text readable whatever the scenery, as XAG 102 recommends.

**Who it is for.** Players with low vision or color blindness, and those bothered by busy scenery.

**Recommendations.** GAG basic *"Ensure no essential information is conveyed by a fixed colour alone"*, GAG intermediate *"Provide an option to adjust contrast"*.

### 3.7 The window: background color and frame rate

**What it is.** `Window` has a background color (`clearScreenColor`) and a frame rate cap (`fps`, 120 by default). Both can change during the game.

**Why it is good.**

- **A plain background** of the player's choice, for a high contrast mode.
- **Adjustable FPS**: on a low-end machine, lowering the FPS keeps the game smooth and steady. The game keeps the same speed, since movement depends on elapsed time (`Clock`), not on the number of frames.

**Who it is for.** Players who need a background without details, and those who play on a modest computer.

---

## 4. Summary

| Feature | Why it is good | Who for | GAG level |
|---|---|---|---|
| Rebinding keys (`Keybind`) | Everyone puts the controls where they can reach them, without changing the game code | One hand, reduced dexterity, left-handed | Basic |
| AZERTY and QWERTY layouts | Correct key names and default keys suited to the keyboard | Non-French keyboards | — |
| Pressed, held, released | Auto-fire, toggle, action on release | Reduced dexterity, pain, fatigue | Intermediate |
| Mouse | A second device to play with | One hand, adapted mice | Intermediate |
| Text size, color, spacing and font | Readable, adjustable text | Low vision, dyslexia | Basic to advanced |
| Image tint and scale | Dimmed scenery, chosen colors, enlarged elements | Low vision, color blindness | Basic to intermediate |
| Background color, FPS | High contrast, smooth game on a small machine | High contrast, modest machines | — |

---

## 5. Accessibility of the documentation

The project's documentation follows a few rules to stay readable by everyone, including with a screen reader:

- **Two languages**: every document exists in French and English, with a link to the other version at the top of the page.
- **Hierarchical headings** and a table of contents, to navigate from heading to heading.
- **Simple tables**, with a header row and no merged cells.
- **No information carried by an icon or a color alone**: tables spell out "Yes" and "No".
- **Links that say where they lead** ("the game making guide" rather than "here").
- **Code in code blocks**, which screen readers announce as such.

**Who it is for.** People who use a screen reader, non-French speakers, and anyone looking for a precise piece of information quickly.

---

## 6. Sources

- Game Accessibility Guidelines, full list: <https://gameaccessibilityguidelines.com/full-list/>
- Microsoft, *Xbox Accessibility Guidelines*: <https://learn.microsoft.com/en-us/gaming/accessibility/guidelines>
- Microsoft, XAG 101, text display: <https://learn.microsoft.com/en-us/gaming/accessibility/xbox-accessibility-guidelines/101>
- Microsoft, XAG 102, contrast: <https://learn.microsoft.com/en-us/gaming/accessibility/xbox-accessibility-guidelines/102>
- Microsoft, XAG 107, input: <https://learn.microsoft.com/en-us/gaming/accessibility/xbox-accessibility-guidelines/107>
- Colour Blind Awareness, *Colour Blindness*: <https://www.colourblindawareness.org/colour-blindness/>
