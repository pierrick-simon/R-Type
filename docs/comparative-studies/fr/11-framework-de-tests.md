# 11 · Quel framework de tests ?

> 🌐 **Langue :** Français · [English](../en/11-test-framework.md)

> **Décision :** les tests unitaires du client et du serveur (et, à terme, du moteur) utilisent **Criterion**, à activer aussi sous Windows.
> **Statut :** adopté · **Date :** octobre 2026 · [Retour au sommaire](README.md)

## En bref

Le projet utilise **Criterion** : il est déjà intégré au build et à l'intégration continue, connu de l'équipe, et il exécute chaque test dans son propre processus, ce qui permet de détecter proprement un plantage. Il fonctionne sous Windows (MinGW et Visual Studio 2015+), où ses auteurs le testent en continu. Il reste une étape : notre `CMakeLists.txt` le désactive sous Windows, et il faut l'y compiler avec Meson, car il n'existe ni binaire Windows, ni paquet vcpkg ou Conan. **GoogleTest**, le framework C++ le plus répandu, est l'alternative si cette installation devient un frein.

---

## Contexte

État actuel du dépôt :

- **Le client et le serveur** ont chacun une cible de tests Criterion (`client_tests_run`, `server_tests_run`), avec la couverture de code (`--coverage`).
- **Le `CMakeLists.txt` racine** cherche une installation de Criterion déjà présente sur la machine (`find_library`), et **seulement hors Windows** (`if (NOT WIN32 ...)`).
- **L'intégration continue** installe Criterion avec `apt-get install libcriterion-dev`.
- **Le Poc** (`Games/Poc`) a son propre lanceur de tests, plus de 200 tests sans dépendance. C'est un programme de validation de l'ECS : il n'est pas concerné par ce choix.

Les tests vont devenir indispensables avec le réseau ([étude 10](10-protocole-reseau.md)) : lecture et écriture des en-têtes, connexion, contrôle de présence. Ce code doit être testé **sous Windows comme sous Linux**, car c'est là qu'apparaissent les différences entre systèmes et compilateurs (taille des types, comportement des sockets).

---

## Critères

| Critère | Poids | Pourquoi il compte |
|---|---|---|
| **Connaissance de l'équipe** | 3 | Un outil déjà maîtrisé permet d'écrire des tests tout de suite |
| **Isolation des plantages** | 2 | Le moteur manipule de la mémoire et du réseau : un test qui plante ne doit pas masquer les autres |
| **Déjà en place** | 2 | Build, couverture de code et intégration continue sont déjà configurés |
| **Windows (MSVC) et Linux** | 2 | Les deux cibles du projet ([étude 01](01-systemes-exploitation.md)) |
| **Installation et intégration CMake** | 1 | Récupéré sans installation manuelle ([étude 09](09-gestion-des-dependances.md)) |
| **Adoption et documentation** | 1 | Exemples, réponses, intégration dans les IDE |
| **Fonctionnalités** | 1 | Fixtures, tests paramétrés, objets simulés (*mocks*) |
| **Temps de compilation** | 1 | Les tests se recompilent souvent |

---

## Les données

### Ce qu'utilisent les développeurs C++

L'enquête *The State of C++ 2025* de JetBrains s'appuie sur les réponses de 1 800 développeurs C++ de 22 pays, issues de sa *Developer Ecosystem Survey 2025*. JetBrains précise que les résultats peuvent être légèrement biaisés en faveur des utilisateurs de ses produits ([JetBrains, *The State of C++ 2025*](https://lp.jetbrains.com/the-state-of-cpp-2025/)). Frameworks de tests unitaires utilisés :

| Framework | Part des développeurs C++ |
|---|---|
| **GoogleTest** | **31 %** |
| CppUnit | 8 % |
| Boost.Test | 7 % |
| Catch2 | 7 % |
| CppUTest | 5 % |
| doctest | 4 % |
| Autre | 2 % |
| Tests écrits sans framework | 17 % |
| Pas de tests unitaires en C++ | 37 % |

Criterion n'apparaît pas dans la liste : il est moins répandu, mais ce critère pèse peu pour une équipe qui le connaît déjà.

### Popularité et activité (GitHub, 19 octobre 2026)

| Framework | Étoiles | Licence | Dernière version |
|---|---|---|---|
| GoogleTest | 39 655 | BSD-3-Clause | v1.18.0 (août 2026) |
| Catch2 | 21 536 | BSL-1.0 | v3.16.1 (octobre 2026) |
| doctest | 6 900 | MIT | v2.5.3 (juillet 2026) |
| Criterion | 2 308 | MIT | v2.5.0 (octobre 2026) |

Les quatre sont maintenus activement : Criterion a publié sa dernière version ce mois-ci.

### Criterion

- **Multiplateforme :** Criterion « *fonctionne sous Linux, FreeBSD, macOS et Windows (compilé avec MinGW GCC et Visual Studio 2015+)* », et s'utilise en C comme en C++ ([Criterion](https://github.com/Snaipe/Criterion)).
- **Testé sous Windows :** son intégration continue compile et teste la bibliothèque sur `windows-latest`, en plus de Linux et macOS ([Criterion, `ci.yml`](https://github.com/Snaipe/Criterion/blob/bleeding/.github/workflows/ci.yml)).
- **Isolation :** chaque test s'exécute dans son propre processus. Un plantage est signalé comme l'échec de ce test, sans arrêter les autres, et peut même être testé volontairement ([Criterion](https://github.com/Snaipe/Criterion)).
- **Installation sous Windows :** les versions publiées ne fournissent un binaire que pour Linux x86-64 ([Criterion, *Releases*](https://github.com/Snaipe/Criterion/releases)), et Criterion n'est ni dans vcpkg, ni dans Conan Center (vérifié le 19 octobre 2026). Il faut donc le compiler, avec **Meson, Ninja, pkg-config, libffi et libgit2** ([Criterion, *Setup*](https://criterion.readthedocs.io/en/master/setup.html)). Pour le faire, l'intégration continue de Criterion installe Ninja, CMake et Python avec Chocolatey.

### Les autres frameworks

- **GoogleTest :** sa documentation officielle montre l'intégration par `FetchContent`, puis `gtest_discover_tests` pour que `ctest` découvre chaque test. Il demande **C++17 au minimum** ([GoogleTest, *Quickstart: CMake*](https://google.github.io/googletest/quickstart-cmake.html)). Il fournit aussi GoogleMock (objets simulés) et des tests de plantage (*death tests*).
- **Catch2 v3 :** n'est plus un simple en-tête, mais « *une bibliothèque normale, avec plusieurs en-têtes et une implémentation compilée à part* ». Il vise C++14 et plus, et offre des sections, des macros de style BDD et des micro-benchmarks ([Catch2](https://github.com/catchorg/Catch2)).
- **doctest :** un seul en-tête, qui se présente comme « *de loin le plus rapide à la compilation* » parmi les frameworks complets ([doctest](https://github.com/doctest/doctest)). Ses benchmarks (GCC, 2018) donnent par exemple **11 s contre 99 s** pour inclure l'en-tête dans 200 fichiers, face à Catch 2. Catch2 v3, désormais compilé à part, n'y figure pas ([doctest, *Benchmarks*](https://github.com/doctest/doctest/blob/master/doc/markdown/benchmarks.md)).

---

## Les options étudiées

### Criterion (retenu)

**Points forts**

- **Déjà en place :** cibles de tests, couverture de code et intégration continue sont configurées.
- **Connu de l'équipe :** syntaxe simple (`Test(suite, nom)`, `cr_assert`), utilisable tout de suite.
- **Isolation des tests** dans des processus séparés : un plantage est signalé proprement, sans masquer les autres tests.
- **Fonctionne sous Windows**, et y est testé par ses auteurs.

**Points faibles**

- **Sous Windows, il faut le compiler** avec Meson et ses dépendances, sur chaque poste et dans l'intégration continue.
- **Pas de récupération par `FetchContent`**, puisqu'il ne se construit pas avec CMake.
- **Peu répandu** en C++ (absent de l'enquête JetBrains) : moins d'aide et d'intégration dans les IDE.

### GoogleTest

**Points forts**

- **Le plus utilisé**, de loin, avec la meilleure intégration dans les IDE (CLion, Visual Studio, VS Code) et dans `ctest`.
- **`FetchContent` et MSVC**, documentés officiellement : rien à installer.
- **Le plus complet :** fixtures, tests paramétrés, GoogleMock, tests de plantage.

**Points faibles**

- **Une migration :** cibles, intégration continue et syntaxe à changer, et un outil nouveau pour l'équipe.
- **Pas d'isolation systématique** des plantages : il faut écrire des *death tests* pour tester un plantage.

### Catch2 v3

**Points forts**

- **Syntaxe très lisible** (sections, `REQUIRE(a == b)` avec affichage des valeurs), `FetchContent`, MSVC.

**Points faibles**

- **Une migration**, et moins répandu que GoogleTest (7 % contre 31 %).

### doctest

**Points forts**

- **Le plus rapide à compiler**, un seul en-tête.

**Points faibles**

- **Une migration**, peu répandu (4 %), moins de fonctionnalités avancées.

### Boost.Test

**Points faibles**

- **Dépend de Boost**, que le projet a justement évité ([étude 06](06-bibliotheque-reseau.md)).

### Un lanceur maison (comme le Poc)

**Points forts**

- Aucune dépendance, fonctionne partout.

**Points faibles**

- **Tout est à écrire :** rapports, filtres, intégration à `ctest` et aux IDE, fixtures. Suffisant pour le Poc, pas pour tout le projet.

---

## Grille de comparaison

| Critère (poids) | Criterion | GoogleTest | Catch2 v3 | doctest | Boost.Test | Lanceur maison |
|---|---|---|---|---|---|---|
| Connaissance de l'équipe (3) | 5 | 3 | 2 | 2 | 2 | 4 |
| Isolation des plantages (2) | 5 | 4 | 2 | 2 | 2 | 1 |
| Déjà en place (2) | 5 | 2 | 2 | 2 | 2 | 2 |
| Windows (MSVC) et Linux (2) | 4 | 5 | 5 | 5 | 4 | 5 |
| Installation et intégration CMake (1) | 2 | 5 | 5 | 5 | 2 | 5 |
| Adoption, documentation (1) | 1 | 5 | 3 | 2 | 3 | 1 |
| Fonctionnalités (1) | 3 | 5 | 4 | 3 | 4 | 1 |
| Temps de compilation (1) | 4 | 3 | 3 | 5 | 3 | 5 |
| **Total pondéré /100** | **82** | **75** | **60** | **60** | **52** | **62** |

---

## Décision et conséquences

**Le projet garde Criterion.** Pour qu'il couvre aussi Windows :

1. **Retirer la condition `NOT WIN32`** du bloc Criterion du `CMakeLists.txt` racine, pour que les tests soient cherchés et compilés sous Windows aussi.
2. **Documenter l'installation sous Windows :** compiler Criterion avec Meson et Ninja, puis l'installer là où `find_library` le trouvera (ou indiquer son chemin à CMake).
3. **Ajouter une compilation Windows à l'intégration continue**, qui compile Criterion comme le fait sa propre intégration continue, puis lance les tests.

**Ce que l'on accepte :** une installation plus lourde sous Windows que celle d'un framework récupérable par `FetchContent`, et un framework moins répandu que GoogleTest.

**Le Poc garde son lanceur maison :** c'est un programme de démonstration et de validation de l'ECS, autonome par conception.

---

## Quand revoir ce choix

- **L'installation de Criterion sous Windows devient un frein** pour l'équipe ou l'intégration continue : passer à **GoogleTest**, récupéré par `FetchContent`. Il suffit alors de remplacer `Test(...)` par `TEST(...)` et `cr_assert` par `EXPECT_TRUE`, et d'appeler `gtest_discover_tests`.
- **Les tests ont besoin d'objets simulés** (par exemple un faux socket pour tester le réseau sans connexion) : GoogleMock, fourni avec GoogleTest, devient un vrai argument.
- **Criterion arrive dans vcpkg ou Conan, ou publie des binaires Windows** : son installation devient aussi simple que celle des autres.

---

## Sources

- JetBrains, *The State of C++ 2025* : <https://lp.jetbrains.com/the-state-of-cpp-2025/>
- Criterion : <https://github.com/Snaipe/Criterion>
- Criterion, intégration continue (`ci.yml`) : <https://github.com/Snaipe/Criterion/blob/bleeding/.github/workflows/ci.yml>
- Criterion, versions publiées : <https://github.com/Snaipe/Criterion/releases>
- Criterion, *Setup* : <https://criterion.readthedocs.io/en/master/setup.html>
- GoogleTest, *Quickstart: Building with CMake* : <https://google.github.io/googletest/quickstart-cmake.html>
- Catch2 : <https://github.com/catchorg/Catch2>
- doctest : <https://github.com/doctest/doctest>
- doctest, *Benchmarks* : <https://github.com/doctest/doctest/blob/master/doc/markdown/benchmarks.md>
- vcpkg, catalogue des ports : <https://github.com/microsoft/vcpkg/tree/master/ports>
- Conan Center Index, catalogue des recettes : <https://github.com/conan-io/conan-center-index/tree/master/recipes>
- API GitHub (étoiles, licences et dernières versions), consultée le 19 octobre 2026
