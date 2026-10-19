# 09 · Comment récupérer les dépendances ?

> 🌐 **Langue :** Français · [English](../en/09-dependency-management.md)

> **Décision :** les dépendances (raylib, raylib-cpp, Asio, plf::colony) sont téléchargées par **`FetchContent`**, intégré à CMake, avec des versions figées.
> **Statut :** adopté · **Date :** octobre 2026 · [Retour au sommaire](README.md)

## En bref

`FetchContent` ne demande **rien d'autre que CMake**. Il fonctionne à l'identique sous Windows et Linux, récupère nos quatre bibliothèques et fige leurs versions dans un seul fichier. Son défaut est de tout retélécharger et recompiler dans chaque nouveau dossier de build. **CPM** obtient le même score : c'est une surcouche de `FetchContent` qui ajoute un cache partagé, et donc l'évolution naturelle si la configuration devient trop longue. **vcpkg** est le meilleur des gestionnaires de paquets (cache binaire, toutes nos bibliothèques), mais demande une installation à chaque poste. **Conan** ne propose pas deux de nos quatre bibliothèques.

---

## Contexte

Le moteur dépend de quatre bibliothèques externes, déclarées dans `Engine/CMakeLists.txt` :

| Bibliothèque | Version utilisée | Type |
|---|---|---|
| raylib | `5.5` | Compilée (bibliothèque partagée) |
| raylib-cpp | `v5.5.0` | En-têtes seulement |
| Asio | `asio-1-34-2` | En-têtes seulement |
| plf::colony | un commit précis | En-tête seulement |

Il faut que n'importe quel membre de l'équipe, sous Windows ou Linux, puisse cloner le dépôt et compiler **sans installer ces bibliothèques à la main**, et que tout le monde compile **exactement les mêmes versions**. L'[étude 03](03-systeme-de-build.md) a retenu CMake ; celle-ci compare les façons de récupérer les dépendances avec lui.

---

## Critères

| Critère | Poids | Pourquoi il compte |
|---|---|---|
| **Rien à installer** | 3 | Un nouveau poste doit compiler avec seulement CMake et un compilateur |
| **Windows et Linux** | 3 | [Étude 01](01-systemes-exploitation.md) |
| **Nos 4 bibliothèques disponibles** | 3 | Une bibliothèque absente oblige à écrire et maintenir sa propre recette |
| **Versions figées** | 2 | Tout le monde compile le même code, et un build ancien se reproduit |
| **Poids du dépôt, facilité des mises à jour** | 2 | Ne pas gonfler le dépôt Git, changer de version en une ligne |
| **Temps de configuration** | 2 | Un nouveau dossier de build ne doit pas coûter de longues minutes |
| **Aucune couche en plus** | 1 | Moins d'outils, c'est moins de pannes |
| **Hors ligne** | 1 | Pouvoir reconfigurer sans connexion |

---

## Les données

### Ce qu'utilisent les développeurs C++

Selon l'enquête JetBrains 2025 (1 800 développeurs C++), la plupart gèrent leurs bibliothèques avec **le gestionnaire de paquets du système (36 %)** ou **en incluant leurs sources dans le build (35 %)**. Les gestionnaires dédiés restent minoritaires : **vcpkg 11 %**, **NuGet 7 %**, **Conan 6 %**, chiffre donné dans le texte du rapport ([JetBrains, *The State of C++ 2025*](https://lp.jetbrains.com/the-state-of-cpp-2025/)). `FetchContent` et CPM relèvent de la deuxième catégorie : les sources sont téléchargées puis compilées avec le projet.

### Nos bibliothèques sont-elles disponibles ?

Vérification dans les catalogues officiels, le 19 octobre 2026 :

| Bibliothèque | vcpkg ([ports](https://github.com/microsoft/vcpkg/tree/master/ports)) | Conan Center ([recettes](https://github.com/conan-io/conan-center-index/tree/master/recipes)) |
|---|---|---|
| raylib | Oui (6.0) | Oui (6.0, 5.5…) |
| raylib-cpp | Oui (6.0.3) | Non |
| Asio | Oui (1.32.0) | Oui (1.38.2, 1.36.0…) |
| plf::colony | Oui (7.7.15) | Non |

- **Conan Center ne propose ni raylib-cpp, ni plf::colony** : il faudrait écrire et maintenir nos propres recettes.
- **vcpkg les propose toutes**, mais ses versions par défaut diffèrent des nôtres (raylib 6.0 au lieu de 5.5, Asio 1.32 au lieu de 1.34.2). Il faudrait figer chaque version à la main dans le manifeste.

### Ce que demande chaque outil

- **vcpkg :** cloner son dépôt, lancer un script d'amorçage (`bootstrap-vcpkg.sh` ou `.bat`), définir `VCPKG_ROOT`, puis passer son fichier de *toolchain* à CMake. Son catalogue compte plus de 2 000 bibliothèques ([Microsoft, *Install and use packages with CMake*](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started)). Son point fort est le **cache binaire, activé par défaut** : une bibliothèque déjà compilée est restaurée « *en quelques secondes* » au lieu d'être recompilée ([Microsoft, *Binary Caching*](https://learn.microsoft.com/en-us/vcpkg/users/binarycaching)).
- **Conan 2 :** s'installe de préférence avec `pip`, et demande **Python 3.8 ou plus**. Des installateurs autonomes existent aussi ([Conan, *Installation*](https://docs.conan.io/2/installation.html)).
- **CPM :** un seul fichier CMake à ajouter au dépôt. Il se décrit comme « *une fine surcouche du module FetchContent de CMake, qui ajoute le contrôle de version, un cache et une API simple* ». Avec `CPM_SOURCE_CACHE`, les sources téléchargées sont partagées entre tous les dossiers de build, et le projet peut ensuite être configuré **hors ligne** ([CPM.cmake](https://github.com/cpm-cmake/CPM.cmake)).
- **`FetchContent` :** intégré à CMake depuis la version 3.11, rien à installer ([CMake, *FetchContent*](https://cmake.org/cmake/help/latest/module/FetchContent.html)).

### Popularité (GitHub, 19 octobre 2026)

| Outil | Étoiles | Licence |
|---|---|---|
| vcpkg | 27 526 | MIT |
| Conan | 9 540 | MIT |
| CPM.cmake | 4 146 | MIT |

`FetchContent` fait partie de CMake : il n'a pas de dépôt à part.

---

## Les options étudiées

### `FetchContent` (retenu)

**Points forts**

- **Rien à installer**, identique sous Windows et Linux.
- **Les versions sont figées** par `GIT_TAG`, au même endroit (`Engine/CMakeLists.txt`). Changer de version, c'est modifier une ligne.
- **Aucune recette à écrire** : n'importe quel dépôt Git fait l'affaire, même s'il n'existe dans aucun catalogue.

**Points faibles**

- **Chaque nouveau dossier de build retélécharge et recompile tout**, raylib compris.
- **Une connexion est nécessaire** à la première configuration.

### CPM.cmake

**Points forts**

- **Tous les avantages de `FetchContent`**, plus un cache de sources partagé et la configuration hors ligne.

**Points faibles**

- **Un fichier de plus** (`CPM.cmake`), et chaque développeur doit définir `CPM_SOURCE_CACHE` pour profiter du cache.
- Le cache porte sur les **sources** : raylib est quand même recompilée dans chaque dossier de build.

### vcpkg (mode manifeste)

**Points forts**

- **Cache binaire par défaut** : raylib n'est compilée qu'une fois par machine.
- **Les quatre bibliothèques sont disponibles.**

**Points faibles**

- **À installer sur chaque poste** et dans l'intégration continue (clone, amorçage, variable d'environnement, *toolchain*).
- **Versions à figer à la main** dans le manifeste, car elles diffèrent de celles du projet.

### Conan 2

**Points forts**

- Très puissant (profils, compilation croisée, binaires préconstruits).

**Points faibles**

- **Python ou un installateur à part**, sur chaque poste.
- **raylib-cpp et plf::colony absentes** du catalogue : deux recettes à écrire.

### Paquets du système (`apt`, etc.)

**Points forts**

- Le plus simple sous Linux, et très rapide (binaires déjà compilés).

**Points faibles**

- **N'existe pas sous Windows.**
- **Les versions dépendent de la distribution** : deux membres de l'équipe peuvent compiler avec des versions différentes.

### Copier les sources dans le dépôt, ou sous-modules Git

**Points forts**

- **Hors ligne, versions figées, rien à installer.**

**Points faibles**

- **Copier les sources gonfle le dépôt** : le dépôt de raylib, avec ses exemples et ses ressources, pèse plusieurs dizaines de mégaoctets. Les mises à jour se font à la main.
- **Les sous-modules** évitent la copie, mais demandent `git submodule update --init`, une étape qu'on oublie facilement.

---

## Grille de comparaison

| Critère (poids) | FetchContent | CPM | vcpkg | Conan 2 | Paquets système | Sources copiées | Sous-modules |
|---|---|---|---|---|---|---|---|
| Rien à installer (3) | 5 | 4 | 2 | 2 | 3 | 5 | 4 |
| Windows et Linux (3) | 5 | 5 | 5 | 5 | 1 | 5 | 5 |
| Nos 4 bibliothèques (3) | 5 | 5 | 4 | 2 | 2 | 5 | 5 |
| Versions figées (2) | 5 | 5 | 4 | 4 | 2 | 5 | 5 |
| Poids du dépôt, mises à jour (2) | 5 | 5 | 5 | 5 | 4 | 1 | 3 |
| Temps de configuration (2) | 2 | 3 | 5 | 4 | 5 | 3 | 3 |
| Aucune couche en plus (1) | 5 | 4 | 3 | 2 | 5 | 5 | 4 |
| Hors ligne (1) | 2 | 4 | 4 | 4 | 5 | 5 | 4 |
| **Total pondéré /100** | **89** | **89** | **80** | **69** | **59** | **86** | **85** |

---

## Décision et conséquences

**Le projet garde `FetchContent`.** CPM obtient le même score : les deux se valent sur le fond, et `FetchContent` est déjà en place, sans fichier ni réglage supplémentaire.

**Bonnes pratiques à garder :**

- **Toujours figer une version précise** (`GIT_TAG` sur un tag ou un commit, jamais sur une branche comme `master`), comme c'est déjà le cas pour les quatre bibliothèques.
- **Utiliser `GIT_SHALLOW TRUE`** pour ne télécharger que la version demandée, sans tout l'historique (déjà fait pour Asio, raylib et raylib-cpp).
- **Réutiliser le même dossier `build/`** plutôt que d'en créer un nouveau, pour éviter de tout retélécharger.

**Ce que l'on accepte :** une première configuration longue (téléchargement et compilation de raylib), et une connexion Internet à ce moment-là.

---

## Quand revoir ce choix

- **L'équipe crée souvent de nouveaux dossiers de build** (plusieurs configurations, Debug et Release) : passer à **CPM** avec `CPM_SOURCE_CACHE`. La migration est légère, puisque CPM repose sur `FetchContent`.
- **La compilation de raylib dans l'intégration continue devient trop longue** : passer à **vcpkg**, dont le cache binaire évite de la recompiler à chaque exécution.
- **Une dépendance disparaît de GitHub** : copier ses sources dans le dépôt.

---

## Sources

- JetBrains, *The State of C++ 2025* : <https://lp.jetbrains.com/the-state-of-cpp-2025/>
- CMake, *FetchContent* : <https://cmake.org/cmake/help/latest/module/FetchContent.html>
- CMake, notes de version 3.11 (arrivée de `FetchContent`) : <https://cmake.org/cmake/help/latest/release/3.11.html>
- CPM.cmake : <https://github.com/cpm-cmake/CPM.cmake>
- Microsoft, *Install and use packages with CMake* (vcpkg) : <https://learn.microsoft.com/en-us/vcpkg/get_started/get-started>
- Microsoft, *Binary Caching* (vcpkg) : <https://learn.microsoft.com/en-us/vcpkg/users/binarycaching>
- vcpkg, catalogue des ports : <https://github.com/microsoft/vcpkg/tree/master/ports>
- Conan, *Installation* : <https://docs.conan.io/2/installation.html>
- Conan Center Index, catalogue des recettes : <https://github.com/conan-io/conan-center-index/tree/master/recipes>
- API GitHub (étoiles et licences), consultée le 19 octobre 2026
