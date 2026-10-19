# 03 · Pourquoi CMake ?

> 🌐 **Langue :** Français · [English](../en/03-build-system.md)

> **Décision :** le projet se construit avec **CMake**, et ses dépendances sont téléchargées par **`FetchContent`**.
> **Statut :** adopté · **Date :** octobre 2026 · [Retour au sommaire](README.md)

## En bref

Un `Makefile` ne fonctionne pas tel quel avec le compilateur de Windows (MSVC), et ne sait pas télécharger les dépendances. **CMake** décrit le projet une seule fois, puis génère des Makefiles ou des fichiers Ninja sous Linux, et une solution Visual Studio sous Windows. C'est l'outil de build C++ **le plus utilisé** (59 à 79 % des développeurs selon les enquêtes). `FetchContent` télécharge raylib, Asio et plf::colony à la configuration, avec une version figée : il suffit de CMake et d'un compilateur pour tout construire.

---

## Contexte

Le projet compte trois bibliothèques (`engine-system`, `engine-graphical`, `engine-network`), trois exécutables (client, serveur, Poc), des tests, et quatre dépendances externes (raylib, raylib-cpp, Asio, plf::colony). Il doit se construire sous **Linux (GCC, Clang)** et sous **Windows (MSVC)** ([étude 01](01-systemes-exploitation.md)).

Un outil de build doit donc :

1. **compiler** les sources dans le bon ordre, en ne recompilant que ce qui a changé ;
2. **fonctionner sur les deux systèmes**, avec leurs compilateurs respectifs ;
3. **récupérer les dépendances**, sans demander à chaque membre de l'équipe de les installer à la main.

---

## Critères

| Critère | Poids | Pourquoi il compte |
|---|---|---|
| **Multiplateforme (dont MSVC)** | 3 | Un seul fichier de build pour Windows et Linux |
| **Gestion des dépendances** | 3 | raylib, Asio, plf::colony doivent arriver sans installation manuelle |
| **Adoption, documentation, IDE** | 2 | Aide en ligne, intégration dans CLion, Visual Studio, VS Code |
| **Simplicité** | 2 | Tout le monde doit pouvoir ajouter un fichier ou une cible |
| **Vitesse de build** | 1 | Compilation incrémentale rapide |

---

## Les données

### Ce qu'utilisent les développeurs C++

**Enquête JetBrains 2025** (1 800 développeurs C++), outils de build utilisés régulièrement ([JetBrains, *The State of C++ 2025*](https://lp.jetbrains.com/the-state-of-cpp-2025/)) :

| Outil | Part |
|---|---|
| **CMake** | **59 %** |
| Projets Visual Studio (MSBuild) | 34 % |
| Makefiles | 27 % |
| Ninja | 15 % |

**Enquête ISO C++ 2021** (1 853 réponses, plusieurs choix possibles) ([Standard C++ Foundation](https://isocpp.org/files/papers/CppDevSurvey-2021-04-summary.pdf)) :

| Outil | Part |
|---|---|
| **CMake** | **79,38 %** |
| Make / nmake | 41,23 % |
| MSBuild | 38,96 % |
| Ninja | 33,35 % |
| Meson | 4,48 % |
| Bazel | 3,78 % |

Les deux enquêtes n'interrogent pas les mêmes personnes, d'où l'écart, mais elles disent la même chose : **CMake est le standard de fait**, loin devant Meson ou Bazel.

### Ce que fait CMake, exactement

CMake ne compile rien lui-même : c'est un **générateur**. Selon sa documentation, « *un générateur CMake est chargé d'écrire les fichiers d'entrée d'un système de build natif* » ([CMake, *cmake-generators*](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html)). À partir des mêmes `CMakeLists.txt`, il produit :

- des **Makefiles** ou des fichiers **Ninja** sous Linux ;
- une **solution Visual Studio** (MSBuild) sous Windows ;
- un projet **Xcode** sous macOS.

Make et CMake ne sont donc pas vraiment concurrents : sous Linux, `cmake --build build` lance `make`. CMake ajoute la couche qui rend le projet portable.

### Les dépendances

Selon la même enquête JetBrains 2025, la plupart des développeurs C++ gèrent leurs bibliothèques avec le **gestionnaire de paquets du système (36 %)** ou en **incluant les sources dans le build (35 %)**. Les gestionnaires dédiés restent minoritaires : **vcpkg 11 %**, **Conan 6 %** (chiffre donné dans le texte du rapport) ([JetBrains](https://lp.jetbrains.com/the-state-of-cpp-2025/)).

---

## Les options étudiées

### CMake + `FetchContent` (retenu)

**Points forts**

- **Un seul fichier pour Windows et Linux**, avec l'outil le plus répandu.
- **`FetchContent` télécharge les dépendances à la configuration** et les compile avec le projet. Les versions sont figées dans `Engine/CMakeLists.txt` : `asio-1-34-2`, raylib `5.5`, raylib-cpp `v5.5.0`, et un commit précis pour plf::colony. Tout le monde compile exactement le même code.
- **Rien à installer en plus** de CMake et d'un compilateur : ni Python (Conan), ni outil à amorcer (vcpkg).
- **Intégration native** dans CLion, Visual Studio et VS Code. `compile_commands.json` (activé par `CMAKE_EXPORT_COMPILE_COMMANDS`) alimente `clang-tidy` et l'autocomplétion.

**Points faibles**

- **Le langage CMake est verbeux** et a beaucoup évolué : un tutoriel ancien peut enseigner de mauvaises pratiques (variables globales plutôt que cibles).
- **La première configuration télécharge et compile raylib**, ce qui demande du temps et une connexion Internet. Un nouveau dossier de build recommence le téléchargement.

### Make seul

**Points forts**

- Connu de toute l'équipe, et simple pour un petit projet.

**Points faibles**

- **Ne fonctionne pas avec MSVC tel quel.** GNU Make n'est pas fourni avec Windows, et l'équivalent de Microsoft (`nmake`) a une syntaxe différente. Il faudrait maintenir deux fichiers de build.
- **Aucune gestion des dépendances :** il faudrait installer raylib à la main sur chaque machine, ou écrire ses propres scripts de téléchargement.
- **Dépendances entre fichiers à écrire soi-même** (`-MMD`) pour ne recompiler que le nécessaire.

### Meson

**Points forts**

- Syntaxe plus claire que CMake, build rapide (il s'appuie sur Ninja), système de dépendances intégré (*wraps*).

**Points faibles**

- **Peu répandu** (4,48 % dans l'enquête ISO 2021), et demande Python.

### Bazel

**Points forts**

- Builds reproductibles et très rapides sur d'énormes dépôts (conçu par Google).

**Points faibles**

- **Lourd à mettre en place** pour un projet de cette taille, et peu répandu en C++ (3,78 %).

### Premake, projets Visual Studio

**Points faibles**

- **Premake :** syntaxe Lua agréable, mais peu répandu et sans gestion des dépendances.
- **Projets Visual Studio seuls :** Windows uniquement.

### Et pour les dépendances : Conan, vcpkg, CPM ?

*Résumé ci-dessous ; la comparaison complète est dans l'[étude 09](09-gestion-des-dependances.md).*

| Option | Avantage | Inconvénient |
|---|---|---|
| **`FetchContent` (retenu)** | Intégré à CMake, rien à installer | Recompile les dépendances dans chaque dossier de build |
| **CPM.cmake** | Surcouche de `FetchContent`, avec un cache partagé (`CPM_SOURCE_CACHE`) | Un fichier de plus dans le dépôt |
| **vcpkg** | Catalogue immense, binaires mis en cache | Doit être cloné et amorcé ; 11 % d'adoption |
| **Conan** | Puissant, gère la compilation croisée | Demande Python et une installation séparée ; 6 % d'adoption |
| **Paquets système** (`apt`) | Le plus simple sous Linux | N'existe pas sous Windows, versions différentes selon les distributions |

CPM se présente lui-même comme une surcouche de `FetchContent` ([CPM.cmake](https://github.com/cpm-cmake/CPM.cmake)), donc les deux reposent sur le même mécanisme. Passer de l'un à l'autre est simple si le temps de configuration devient gênant.

---

## Grille de comparaison

| Critère (poids) | CMake | Make | Meson | Bazel | Premake | Visual Studio |
|---|---|---|---|---|---|---|
| Multiplateforme (3) | 5 | 2 | 5 | 4 | 4 | 1 |
| Dépendances (3) | 4 | 1 | 4 | 4 | 1 | 3 |
| Adoption, IDE (2) | 5 | 3 | 2 | 2 | 2 | 4 |
| Simplicité (2) | 3 | 3 | 4 | 1 | 4 | 3 |
| Vitesse de build (1) | 4 | 3 | 5 | 5 | 3 | 4 |
| **Total pondéré /100** | **85** | **44** | **80** | **64** | **55** | **55** |

---

## Décision et conséquences

**Le projet se construit avec CMake, et ses dépendances arrivent par `FetchContent`.** Concrètement :

- **Configurer :** `cmake -S . -B build`, toujours dans un dossier séparé. `cmake .` à la racine pollue le dépôt (voir le [README](../../../README.md)).
- **Ajouter un jeu :** un dossier sous `Games/`, son `CMakeLists.txt` et une ligne `add_subdirectory` ([guide de création d'un jeu](../../GAME_GUIDE.fr.md#2-créer-le-dossier-du-jeu)).
- **Changer de version d'une dépendance :** modifier son `GIT_TAG` dans `Engine/CMakeLists.txt`, un seul endroit.

**Ce que l'on accepte :** une première configuration plus longue (téléchargement et compilation de raylib) et une connexion Internet nécessaire à ce moment-là.

> [!NOTE]
> Le README demande « CMake ≥ 4.4 », mais les `CMakeLists.txt` déclarent `cmake_minimum_required(VERSION 3.25...4.0)` et l'intégration continue installe `>=3.25`. Il faudra harmoniser ces trois valeurs.

---

## Quand revoir ce choix

- **La configuration devient trop longue** pour l'équipe : passer à CPM avec un cache partagé, ou à vcpkg en mode manifeste (binaires mis en cache).
- **Une dépendance n'est plus disponible** sur GitHub (dépôt supprimé ou renommé) : copier ses sources dans le dépôt, ou passer à un gestionnaire de paquets.
- **Le projet ajoute une plateforme** (web, mobile) : vérifier que la dépendance concernée s'y compile avec `FetchContent`.

---

## Sources

- JetBrains, *The State of C++ 2025* : <https://lp.jetbrains.com/the-state-of-cpp-2025/>
- Standard C++ Foundation, *2021 Annual C++ Developer Survey "Lite"* : <https://isocpp.org/files/papers/CppDevSurvey-2021-04-summary.pdf>
- CMake, *cmake-generators(7)* : <https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html>
- CMake, *FetchContent* : <https://cmake.org/cmake/help/latest/module/FetchContent.html>
- CPM.cmake : <https://github.com/cpm-cmake/CPM.cmake>
