# Accessibilité

> 🌐 **Langue :** Français · [English](ACCESSIBILITY.en.md)

Ce document présente ce que le moteur offre pour l'**accessibilité**. Pour chaque fonctionnalité, il explique ce qu'elle permet, pourquoi c'est utile, pour qui, et comment l'utiliser.

- **Public visé :** toute personne qui travaille sur le moteur ou sur un jeu, et qui veut qu'un maximum de joueurs puisse jouer.
- **Prérequis :** le [guide de création d'un jeu](GAME_GUIDE.fr.md) pour l'utilisation des composants. Les exemples de code reprennent ses conventions.

---

## Sommaire

1. [Pourquoi s'en préoccuper](#1-pourquoi-sen-préoccuper)
2. [Les référentiels utilisés](#2-les-référentiels-utilisés)
3. [Les fonctionnalités du moteur](#3-les-fonctionnalités-du-moteur)
4. [Récapitulatif](#4-récapitulatif)
5. [L'accessibilité de la documentation](#5-laccessibilité-de-la-documentation)
6. [Sources](#6-sources)

---

## 1. Pourquoi s'en préoccuper

Un shoot'em up comme R-Type demande beaucoup : des réflexes, de la précision, des appuis répétés, une bonne lecture de l'écran. Chacune de ces exigences peut gêner certains joueurs :

- **Handicap moteur** (dextérité réduite, tremblements, douleurs chroniques, usage d'une seule main) : certaines touches sont difficiles à atteindre, à marteler ou à maintenir.
- **Handicap visuel** (basse vision, daltonisme) : un petit texte ou un élément peu contrasté peut passer inaperçu. D'après Colour Blind Awareness, le daltonisme touche environ **1 homme sur 12 (8 %) et 1 femme sur 200**, le plus souvent pour le rouge et le vert.
- **Handicap cognitif** (difficultés d'attention, de lecture) : un texte peu lisible ou un écran trop chargé fatigue vite.
- **Situations temporaires ou matérielles** : un bras dans le plâtre, un clavier étranger, une machine peu puissante.

Rendre un jeu accessible, c'est surtout **donner des choix** au joueur. Les fonctionnalités ci-dessous en donnent beaucoup, et elles profitent aussi à tous les autres joueurs.

---

## 2. Les référentiels utilisés

Ce document s'appuie sur deux référentiels reconnus dans le jeu vidéo :

- **Game Accessibility Guidelines** (GAG) : une liste de recommandations classées en trois niveaux. Le niveau **basique** est simple à mettre en place et utile à beaucoup de joueurs. Le niveau **intermédiaire** demande plus de travail. Le niveau **avancé** répond à des besoins plus spécifiques ([gameaccessibilityguidelines.com](https://gameaccessibilityguidelines.com/full-list/)).
- **Xbox Accessibility Guidelines** (XAG) : les recommandations de Microsoft, plus chiffrées. Ce document utilise la XAG 101 (affichage du texte), la XAG 102 (contraste) et la XAG 107 (entrées) ([Microsoft Learn](https://learn.microsoft.com/en-us/gaming/accessibility/guidelines)).

Pour chaque fonctionnalité, ce document cite la recommandation à laquelle elle répond, avec son titre d'origine en anglais et son niveau.

---

## 3. Les fonctionnalités du moteur

### 3.1 Réassigner les touches et les boutons (`Keybind`)

**Ce que c'est.** Le composant `Keybind` (`Engine/Graphical/include/Keybind.hpp`) réécrit les entrées du clavier et de la souris **avant** que le jeu ne les lise. Il gère quatre types de correspondances :

| Correspondance | Membre | Exemple |
|---|---|---|
| Touche vers touche | `_keyToKey` | La flèche du haut agit comme Z |
| Bouton de souris vers bouton de souris | `_mouseButtonToMouseButton` | Inverser le clic gauche et le clic droit |
| Touche vers bouton de souris | `_keyToMouseButton` | Espace agit comme un clic gauche |
| Bouton de souris vers touche | `_mouseButtonToKey` | Le clic gauche agit comme Espace |

**Pourquoi c'est bien.**

- **Chacun choisit ses touches** : le joueur place les commandes là où il peut les atteindre confortablement.
- **Jouer d'une seule main** devient possible : tout regrouper d'un côté du clavier, ou mettre le tir sur la souris et le déplacement au clavier.
- **On peut ajouter des touches sans en retirer** : une touche absente du `Keybind` garde son rôle. Faire agir la flèche du haut comme Z laisse Z fonctionner, et le joueur a les deux.
- **Plusieurs touches peuvent mener à la même action** : la flèche du haut et le 8 du pavé numérique vers Z, par exemple.
- **Deux touches peuvent être échangées** : Z vers haut et haut vers Z en même temps. Les correspondances sont calculées à partir des touches réellement pressées, donc l'échange fonctionne.
- **Le code du jeu ne change pas** : le jeu lit `keyDown` et `keyPressed` comme d'habitude, il reçoit simplement les touches déjà réassignées. Un écran de configuration n'a qu'à remplir le `Keybind`.

**Pour qui.** Les joueurs qui ne peuvent utiliser qu'une main (handicap permanent ou blessure temporaire), qui ont une dextérité ou une amplitude de mouvement réduites, les gauchers, et tous ceux qui préfèrent d'autres touches.

**Recommandations.** GAG basique *« Allow controls to be remapped / reconfigured »*. XAG 107 : *« Players should be given the option to remap all of the controls within the game itself »*.

**Comment l'utiliser.** Le `Keybind` se met sur l'entité qui porte le `KeyboardManager` et le `MouseManager`. Son système est enregistré **juste après** les leurs, et **avant** les systèmes du jeu :

```cpp
auto input = gm.createEntity();
gm.addComponent<RType::KeyboardManager>(input);
gm.addComponent<RType::MouseManager>(input);
gm.addComponent<RType::Keybind>(input);

RType::Keybind &bind = gm.getComponent<RType::Keybind>(input);
// Les flèches en plus de ZQSD
bind._keyToKey[RType::Key::Value::Up] = RType::Key::Value::Z;
bind._keyToKey[RType::Key::Value::Left] = RType::Key::Value::Q;
bind._keyToKey[RType::Key::Value::Down] = RType::Key::Value::S;
bind._keyToKey[RType::Key::Value::Right] = RType::Key::Value::D;
// Tirer à la souris
bind._mouseButtonToKey[RType::MouseButton::Value::Left] = RType::Key::Value::Space;

gm.createSystem<RType::KeyboardManager>(RType::KeyboardManager::systemKeyboardManager);
gm.createSystem<RType::MouseManager>(RType::MouseManager::systemMouseManager);
gm.createSystem<RType::Keybind, RType::MouseManager, RType::KeyboardManager>(
    RType::Keybind::systemUpdateKeybind);
// ... puis les systèmes du jeu
```

### 3.2 Les dispositions AZERTY et QWERTY (`Key::Layout`)

**Ce que c'est.** Le `KeyboardManager` prend une disposition de clavier : `Key::Layout::AZERTY` (par défaut) ou `Key::Layout::QWERTY`. Une `Key::Value` désigne **la touche qui porte cette lettre** sur un clavier de la disposition choisie : avec AZERTY, `Key::Value::Z` est la touche marquée Z sur un clavier français, avec QWERTY, celle marquée Z sur un clavier anglais.

```cpp
gm.addComponent<RType::KeyboardManager>(input, RType::Key::Layout::QWERTY);
```

**Pourquoi c'est bien.**

- **Les noms de touches correspondent au clavier du joueur** : « Appuyez sur Z » désigne bien la touche où Z est écrit.
- **Des touches par défaut adaptées à chaque clavier** : combiné au `Keybind`, le jeu peut proposer ZQSD sur un clavier AZERTY et WASD sur un clavier QWERTY, deux groupes de touches à la même place sous la main.
- **La disposition peut changer en cours de partie** : `layout` est un membre public, lu à chaque frame. Un menu d'options peut donc la modifier sans relancer le jeu.

**Pour qui.** Les joueurs dont le clavier n'est pas français, soit la majorité des joueurs dans le monde, et les joueurs francophones qui utilisent un clavier QWERTY.

### 3.3 Touche pressée, maintenue, relâchée

**Ce que c'est.** Le `KeyboardManager` (et le `MouseManager`, pour les boutons) remplit trois listes à chaque frame :

| Liste | Contenu |
|---|---|
| `keyPressed` | Les touches enfoncées **pendant cette frame** |
| `keyDown` | Les touches **maintenues** |
| `keyReleased` | Les touches relâchées **pendant cette frame** |

**Pourquoi c'est bien.** Avec ces trois listes, une même action peut se déclencher de plusieurs façons, selon ce que le joueur peut faire :

- **Tir automatique** : tirer tant que la touche est maintenue (`keyDown`), avec un délai entre deux tirs, au lieu d'un tir par appui.
- **Bascule** : un appui (`keyPressed`) active une action continue, un second appui l'arrête. Le joueur n'a plus rien à maintenir.
- **Action au relâchement** : déclencher une action quand la touche est relâchée (`keyReleased`), ce qui laisse le temps d'annuler un appui involontaire. La XAG 107 le recommande pour les menus.

**Pour qui.** Les joueurs qui ont du mal à marteler une touche ou à la maintenir longtemps : dextérité réduite, douleurs chroniques, fatigue, tremblements.

**Recommandations.** GAG intermédiaire *« Avoid / provide alternatives to requiring buttons to be held down »*. XAG 107 : éviter les appuis répétés rapides et proposer des *« Toggles and "auto" holds »*.

### 3.4 La souris (`MouseManager`)

**Ce que c'est.** Les boutons de la souris (gauche, droit, milieu et quatre boutons latéraux), avec les mêmes listes pressé, maintenu et relâché que le clavier, plus la position (`getPos()`) et la molette (`getWheel()`).

**Pourquoi c'est bien.** La souris est un **second périphérique** pour jouer. Avec le `Keybind`, n'importe quelle action du clavier peut passer sur un bouton de la souris, y compris les boutons latéraux d'une souris de jeu. Le joueur répartit les commandes entre ses deux mains, ou les regroupe sur une seule.

**Pour qui.** Les joueurs plus à l'aise avec une souris qu'avec un clavier, ceux qui jouent d'une main, et ceux qui utilisent une souris adaptée (trackball, souris à gros boutons).

**Recommandation.** GAG intermédiaire *« Support more than one input device »*.

### 3.5 Le texte (`Text2D` et `Font`)

**Ce que c'est.** `Text2D` affiche un texte avec une **taille** (`fontSize`), une **couleur** (`color`), un **espacement entre les lettres** (`spacing`) et une rotation, dans une **police** chargée depuis un fichier (`Font`).

**Pourquoi c'est bien.**

- **Une taille réglable** : `fontSize` peut changer à tout moment, par exemple depuis un réglage « taille du texte ».
- **Une police au choix** : n'importe quelle police TTF ou OTF, par exemple une police sans empattement ou une police pensée pour les lecteurs dyslexiques.
- **Une couleur libre** : un mode « contraste élevé » peut passer le texte en blanc pur ou en jaune vif.
- **Un espacement réglable** entre les lettres, qui facilite la lecture.

**Pour qui.** Les joueurs malvoyants ou dyslexiques, ceux qui jouent loin de l'écran ou sur un petit écran.

**Les valeurs recommandées** (XAG 101 et 102), que ces réglages permettent d'atteindre :

| Réglage | Recommandation |
|---|---|
| Taille minimale par défaut, sur PC | **18 px** de hauteur à 1920×1080 (36 px en 4K) |
| Agrandissement | Jusqu'à **200 %** de la taille minimale |
| Espacement entre les lettres | Au moins **0,12 fois** la taille du texte |
| Contraste du texte normal | Au moins **4,5:1** avec le fond |
| Contraste du texte large (36 px et plus à 1080p, sur PC) | Au moins **3:1** |
| Mode contraste élevé | Au moins **7:1** |

Avec raylib, `fontSize` correspond à peu près à la hauteur du texte en pixels.

**Recommandations.** GAG basique *« Use an easily readable default font size »* et *« Provide high contrast between text/UI and background »*, GAG intermédiaire *« Provide a choice of text colour, low/high contrast choice as a minimum »*, GAG avancé *« Allow the font size to be adjusted »*.

### 3.6 La teinte et la taille des images

**Ce que c'est.** `Sprite2D` et `SpriteSheet2D` ont une **teinte** (`color`, blanc par défaut, c'est-à-dire l'image d'origine) et une **échelle** (`scale`).

**Pourquoi c'est bien.**

- **Assombrir le décor** : teinter le fond en gris foncé (par exemple `raylib::Color{80, 80, 80, 255}`) fait ressortir le vaisseau, les ennemis et les tirs. La XAG 102 cite en exemple un jeu qui permet d'assombrir le décor jusqu'au noir.
- **Laisser le joueur choisir les couleurs** des éléments importants, comme les tirs ennemis, pour qu'il prenne celle qu'il distingue le mieux.
- **Agrandir les éléments du jeu** avec `scale`, pour qu'ils restent visibles.
- **Mettre un fond derrière un texte** : une image d'un pixel blanc, agrandie avec `scale` et teintée en noir semi-transparent, garde le texte lisible quel que soit le décor, comme le recommande la XAG 102.

**Pour qui.** Les joueurs malvoyants ou daltoniens, et ceux que gêne un décor chargé.

**Recommandations.** GAG basique *« Ensure no essential information is conveyed by a fixed colour alone »*, GAG intermédiaire *« Provide an option to adjust contrast »*.

### 3.7 La fenêtre : couleur de fond et images par seconde

**Ce que c'est.** `Window` a une couleur de fond (`clearScreenColor`) et une limite d'images par seconde (`fps`, 120 par défaut). Les deux peuvent changer en cours de partie.

**Pourquoi c'est bien.**

- **Un fond uni** au choix du joueur, pour un mode contraste élevé.
- **Des FPS réglables** : sur une machine peu puissante, une limite plus basse donne un rythme d'images régulier. Les déplacements de `Velocity2D` tiennent compte du temps écoulé (`Clock`) : un objet garde la même vitesse quel que soit le nombre d'images par seconde.

**Pour qui.** Les joueurs qui ont besoin d'un fond sans détails, et ceux qui jouent sur un ordinateur modeste.

---

## 4. Récapitulatif

| Fonctionnalité | Pourquoi c'est bien | Pour qui | Niveau GAG |
|---|---|---|---|
| Réassigner les touches (`Keybind`) | Chacun place les commandes où il peut les atteindre, sans changer le code du jeu | Une main, dextérité réduite, gauchers | Basique |
| Dispositions AZERTY et QWERTY | Noms de touches justes et touches par défaut adaptées au clavier | Claviers non français | — |
| Pressé, maintenu, relâché | Tir automatique, bascule, action au relâchement | Dextérité réduite, douleurs, fatigue | Intermédiaire |
| Souris | Un second périphérique pour jouer | Une main, souris adaptées | Intermédiaire |
| Taille, couleur, espacement et police du texte | Un texte lisible et réglable | Malvoyants, dyslexiques | Basique à avancé |
| Teinte et échelle des images | Décor assombri, couleurs au choix, éléments agrandis | Malvoyants, daltoniens | Basique à intermédiaire |
| Couleur de fond, FPS | Contraste élevé, rythme d'images régulier sur petite machine | Contraste élevé, machines modestes | — |

---

## 5. L'accessibilité de la documentation

La documentation du projet suit quelques règles pour rester lisible par tous, y compris avec un lecteur d'écran :

- **Deux langues** : chaque document existe en français et en anglais, avec un lien vers l'autre version en haut de page.
- **Des titres hiérarchisés** et un sommaire, pour naviguer de titre en titre.
- **Des tableaux simples**, avec une ligne d'en-tête et sans cellules fusionnées.
- **Pas d'information portée par une icône ou une couleur seule** : les tableaux écrivent « Oui » et « Non » en toutes lettres.
- **Des liens qui disent où ils mènent** (« le guide de création d'un jeu » plutôt que « ici »).
- **Le code dans des blocs de code**, que les lecteurs d'écran signalent comme tels.

**Pour qui.** Les personnes qui utilisent un lecteur d'écran, les non-francophones, et tous ceux qui cherchent une information précise rapidement.

---

## 6. Sources

- Game Accessibility Guidelines, liste complète : <https://gameaccessibilityguidelines.com/full-list/>
- Microsoft, *Xbox Accessibility Guidelines* : <https://learn.microsoft.com/en-us/gaming/accessibility/guidelines>
- Microsoft, XAG 101, affichage du texte : <https://learn.microsoft.com/en-us/gaming/accessibility/xbox-accessibility-guidelines/101>
- Microsoft, XAG 102, contraste : <https://learn.microsoft.com/en-us/gaming/accessibility/xbox-accessibility-guidelines/102>
- Microsoft, XAG 107, entrées : <https://learn.microsoft.com/en-us/gaming/accessibility/xbox-accessibility-guidelines/107>
- Colour Blind Awareness, *Colour Blindness* : <https://www.colourblindawareness.org/colour-blindness/>
