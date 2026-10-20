# 05 · Pourquoi raylib ?

> 🌐 **Langue :** Français · [English](../en/05-graphics-library.md)

> **Décision :** l'affichage, l'audio et les entrées du moteur reposent sur **raylib 5.5**, utilisée à travers son interface C++ **raylib-cpp**.
> **Statut :** adopté · **Date :** octobre 2026 · [Retour au sommaire](README.md)

## En bref

raylib est la seule des bibliothèques comparées à offrir **la 2D et la 3D prêtes à l'emploi** (caméras, modèles, shaders), avec **l'audio et les entrées inclus**, **sans aucune dépendance externe**, sur Windows et Linux, sous licence zlib. SFML est plus agréable en C++, mais **refuse explicitement la 3D**. SDL3 et GLFW sont plus bas niveau : il faudrait écrire soi-même tout le rendu. Le prix de raylib : une API en C à état global (une seule fenêtre), un rendu limité à OpenGL, et des conflits de noms avec `windows.h`.

---

## Contexte

Le moteur (`Engine/`) doit afficher des images, des animations et du texte, jouer des sons et lire le clavier et la souris, sur Windows et Linux. Le R-Type est un jeu en 2D, mais le moteur est conçu pour accueillir **d'autres jeux** (le dossier `Games/` est fait pour en contenir plusieurs) et pour permettre des effets que la 2D seule ne donne pas :

- des **décors en 3D** vus de côté, avec une vraie profondeur et de la parallaxe (le style « 2,5D » des R-Type récents) ;
- des **boss modélisés en 3D** dans un jeu en 2D ;
- des **shaders** pour les effets (flash d'impact, distorsion, post-traitement) ;
- des **caméras** (zoom, secousses, suivi).

Un **moteur complet** (Unity, Unreal, Godot) est écarté : il impose sa boucle de jeu, son modèle d'objets et son éditeur, alors que le projet construit son propre moteur, réutilisable, avec son ECS et un serveur qui tourne sans affichage ([contraintes communes](README.md#les-contraintes-communes-à-toutes-les-études)). Il faut donc une **bibliothèque**, pas un moteur.

---

## Critères

| Critère | Poids | Pourquoi il compte |
|---|---|---|
| **2D et 3D intégrées** | 3 | Décors, boss, shaders, caméras ; réutilisation pour d'autres jeux |
| **Simplicité de l'API** | 3 | Toute l'équipe doit pouvoir afficher un sprite rapidement |
| **Windows et Linux** | 2 | [Étude 01](01-systemes-exploitation.md) |
| **Audio et entrées inclus** | 2 | Une seule bibliothèque plutôt que trois à intégrer |
| **Dépendances et intégration CMake** | 2 | Récupérable par `FetchContent` sans installation système ([étude 03](03-systeme-de-build.md)) |
| **Communauté et documentation** | 2 | Exemples, réponses aux questions, maintenance active |
| **Licence** | 1 | Compatible avec un projet distribué |

---

## Les options étudiées

### raylib 5.5 + raylib-cpp (retenu)

**Points forts**

- **2D et 3D dans la même API.** raylib annonce « *un support 3D complet : formes, modèles, billboards, heightmaps et plus* », des shaders de modèles et de post-traitement ([raylib](https://github.com/raysan5/raylib)). Son API comprend `Camera3D`, `BeginMode3D`, `LoadModel`, `DrawModel` et `LoadShader` ([`raylib.h`](https://github.com/raysan5/raylib/blob/master/src/raylib.h)), et les modèles se chargent depuis OBJ, IQM, glTF, VOX et M3D ([`config.h`](https://github.com/raysan5/raylib/blob/master/src/config.h)).
- **Tout est inclus :** fenêtre, rendu, texte, audio (WAV, OGG, MP3, FLAC…), clavier, souris, manette. raylib affirme n'avoir « *aucune dépendance externe : toutes les bibliothèques nécessaires sont incluses* ». C'est ce qui permet de la compiler par `FetchContent`, sans installer quoi que ce soit.
- **Très simple :** une fenêtre et un sprite tiennent en quelques lignes. Le site officiel propose des exemples avec leur code, classés par module (core, formes, textures, texte, modèles 3D, shaders, audio) et par niveau de difficulté ([raylib examples](https://www.raylib.com/examples.html)).
- **La plus populaire** des bibliothèques comparées sur GitHub (voir [Données](#les-données)), et maintenue activement.
- **raylib-cpp** ajoute une couche C++ (classes, RAII, exceptions) : les composants du moteur (`Texture2D`, `Font`, `Sound`) héritent directement de ses classes.

**Points faibles**

- **API en C à état global :** une seule fenêtre, et des fonctions globales (`DrawTexture`, `IsKeyDown`). C'est pour cela que le composant `Window` du moteur refuse d'être créé deux fois.
- **Rendu OpenGL uniquement** (1.1 à 4.3, ES 2.0/3.0) : ni Vulkan, ni DirectX 12, ni Metal. OpenGL suffit largement pour un R-Type, mais il est déprécié sur macOS ([étude 01](01-systemes-exploitation.md#le-cas-de-macos)).
- **Conflits de noms avec `windows.h`** : raylib définit `CloseWindow`, `ShowCursor` ou `Rectangle`, qui existent aussi dans l'API Win32. Inclure les deux dans le même fichier ne compile pas sans précautions ([discussion raylib #1569](https://github.com/raysan5/raylib/discussions/1569)).
- **raylib-cpp est une petite communauté** (moins de 1 000 étoiles) : pour un problème, il faut souvent remonter à la documentation C de raylib.

### SFML 3

**Points forts**

- **API C++ orientée objet**, très agréable (`sf::Sprite`, `sf::Texture`), avec un module réseau et un module audio.

**Points faibles**

- **Pas de 3D, par choix.** La FAQ officielle répond « *Non* » à la question de la 3D : l'équipe veut garder SFML comme une bibliothèque 2D. Pour la 3D, il faut créer le contexte avec SFML puis écrire « *de l'OpenGL brut* » ([FAQ SFML](https://www.sfml-dev.org/faq/general/)).
- **Demande un compilateur C++17** pour SFML 3 ([FAQ SFML](https://www.sfml-dev.org/faq/general/)) : pas un problème ici, puisque le projet est en C++20.

### SDL3

**Points forts**

- **Très répandue** pour la fenêtre, les entrées et l'audio.
- **Une API GPU moderne** (SDL_GPU), au-dessus de Vulkan, Direct3D 12 et Metal ([SDL3 Wiki](https://wiki.libsdl.org/SDL3/CategoryGPU)).

**Points faibles**

- **Bas niveau :** pour la 3D, il faut écrire tout le pipeline (tampons, shaders, matrices). En plus, « *chaque backend (Vulkan, Metal, D3D12) demande un format de shader différent* » ([SDL3 Wiki](https://wiki.libsdl.org/SDL3/CategoryGPU)). C'est un moteur de rendu à écrire avant d'afficher le premier modèle.

### GLFW + OpenGL

**Points forts**

- Contrôle total du rendu, et GLFW est léger et fiable.

**Points faibles**

- **GLFW ne fait que la fenêtre et les entrées.** Il faut ajouter un chargeur OpenGL, une bibliothèque d'images, une bibliothèque audio et un rendu de texte, puis écrire tout l'affichage. raylib utilise d'ailleurs GLFW en interne sur ordinateur ([`rcore_desktop_glfw.c`](https://github.com/raysan5/raylib/blob/master/src/platforms/rcore_desktop_glfw.c)).

---

## Les données

### Popularité et licences (GitHub, 19 octobre 2026)

| Bibliothèque | Étoiles GitHub | Licence | Langage | 3D intégrée |
|---|---|---|---|---|
| **raylib** | **35 321** | zlib | C99 | **Oui** (modèles, caméras, shaders) |
| SDL | 16 834 | zlib | C | Non (API GPU bas niveau) |
| GLFW | 15 407 | zlib | C | Non (fenêtre seulement) |
| SFML | 12 074 | zlib | C++17 | **Non, par choix** |
| raylib-cpp | 970 | zlib | C++ | (interface de raylib) |

*Source : API GitHub, consultée le 19 octobre 2026. Le nombre d'étoiles mesure la popularité, pas la qualité.*

Toutes ces bibliothèques sont sous **licence zlib**, qui autorise un usage commercial et l'édition de liens statique avec du code fermé : la licence ne départage donc pas les options.

### Ce que raylib couvre (selon son README)

| Domaine | Contenu |
|---|---|
| Plateformes | Windows, Linux, macOS, Raspberry Pi, Android, HTML5… |
| Rendu | OpenGL 1.1, 2.1, 3.3, 4.3, ES 2.0, ES 3.0, et un rendu logiciel |
| 3D | Formes, modèles, billboards, heightmaps, shaders de modèle et de post-traitement |
| Audio | Chargement et lecture en streaming : WAV, QOA, OGG, MP3, FLAC, XM, MOD |
| Dépendances | Aucune : tout est inclus |
| Langage | C99, avec des interfaces pour plus de 70 langages |

Source : [README de raylib](https://github.com/raysan5/raylib).

---

## Grille de comparaison

| Critère (poids) | raylib | SFML 3 | SDL3 | GLFW + OpenGL |
|---|---|---|---|---|
| 2D et 3D intégrées (3) | 5 | 1 | 3 | 3 |
| Simplicité de l'API (3) | 4 | 5 | 2 | 1 |
| Windows et Linux (2) | 5 | 5 | 5 | 5 |
| Audio et entrées inclus (2) | 5 | 5 | 4 | 1 |
| Dépendances, CMake (2) | 5 | 4 | 4 | 3 |
| Communauté, documentation (2) | 4 | 4 | 5 | 4 |
| Licence (1) | 5 | 5 | 5 | 5 |
| **Total pondéré /100** | **93** | **79** | **75** | **57** |

---

## Décision et conséquences

**Le moteur utilise raylib 5.5 et raylib-cpp v5.5.0**, récupérées par `FetchContent` dans `Engine/CMakeLists.txt`. raylib est compilée en bibliothèque partagée.

**raylib est enfermée dans `engine-graphical`.** Le code de jeu manipule des composants du moteur (`Sprite2D`, `Text2D`, `Sound`…), pas raylib directement. On pourrait donc changer de bibliothèque en ne réécrivant que `Engine/Graphical`. Le serveur n'affiche rien. Sur la branche `feature/network`, il lie pourtant `engine-graphical`, car le protocole réutilise ses types (touches du clavier, sprites, sons).

**Ce que l'on accepte :**

- **Une seule fenêtre** : c'est une limite de raylib, reprise par le composant `Window`.
- **raylib et `windows.h` ne doivent jamais se croiser dans un même fichier `.cpp`.** Sous Windows, Asio inclut les en-têtes Win32 : les bibliothèques graphique et réseau doivent donc rester séparées, ce que le découpage en `engine-graphical` et `engine-network` doit garantir. Ce n'est pas encore le cas sur la branche `feature/network` ([étude 06](06-bibliotheque-reseau.md#décision-et-conséquences)).
- **Pas de Vulkan, de DirectX ni de Metal** : sans conséquence pour un R-Type.

---

## Quand revoir ce choix

- **Le jeu demande un rendu moderne** (milliers de lumières, rendu différé, Vulkan) : SDL3 et son API GPU deviendraient pertinents, au prix d'un moteur de rendu à écrire.
- **Le projet abandonne toute ambition 3D** et l'équipe préfère une API objet : SFML 3 serait plus confortable.
- **Passer à raylib 6**, déjà disponible (la 6.2 est sortie le 15 octobre 2026 ; vcpkg et Conan proposent la 6.0, et raylib-cpp suit la 6.0 avec sa version 6.0.3) : mettre à jour raylib et raylib-cpp en même temps, car leurs versions sont liées, et vérifier les changements d'API avant de migrer.

---

## Sources

- raylib, README : <https://github.com/raysan5/raylib>
- raylib, cheatsheet : <https://www.raylib.com/cheatsheet/cheatsheet.html>
- raylib, exemples : <https://www.raylib.com/examples.html>
- raylib, discussion #1569 sur les conflits avec `windows.h` : <https://github.com/raysan5/raylib/discussions/1569>
- raylib-cpp : <https://github.com/RobLoach/raylib-cpp>
- SFML, FAQ générale : <https://www.sfml-dev.org/faq/general/>
- SDL3, *CategoryGPU* : <https://wiki.libsdl.org/SDL3/CategoryGPU>
- Star History, comparaison des bibliothèques graphiques : <https://www.star-history.com/compare/graphics-library>
- API GitHub (étoiles et licences), consultée le 19 octobre 2026
