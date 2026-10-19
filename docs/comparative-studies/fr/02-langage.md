# 02 · Pourquoi le C++ ?

> 🌐 **Langue :** Français · [English](../en/02-language.md)

> **Décision :** le moteur et le jeu sont écrits en **C++20**.
> **Statut :** adopté · **Date :** octobre 2026 · [Retour au sommaire](README.md)

## En bref

Le C++ est **le meilleur compromis pour un moteur de jeu temps réel**. Il est dans le peloton de tête en vitesse et en énergie, gère la mémoire sans ramasse-miettes (donc sans pauses imprévisibles), et dispose de l'écosystème jeu le plus riche (raylib, Asio, plf::colony, Unreal, Godot). Ses templates permettent un ECS typé et vérifié à la compilation. Le prix à payer est la **sûreté mémoire**, que le projet compense par le RAII, des handles vérifiés et les sanitizers.

---

## Contexte

Le langage conditionne tout le reste : la vitesse du jeu, les bibliothèques disponibles, le temps de compilation, les bugs possibles. Un jeu temps réel doit produire une image toutes les **16,7 ms** à 60 images par seconde, et le serveur doit simuler la partie et répondre aux clients au même rythme. Le moindre à-coup se voit à l'écran.

---

## Critères

| Critère | Poids | Pourquoi il compte |
|---|---|---|
| **Performance** | 3 | Budget de 16,7 ms par frame, côté client comme serveur |
| **Écosystème jeu** | 3 | Bibliothèques graphiques, réseau, conteneurs ; exemples et documentation |
| **Maîtrise de la mémoire** | 2 | Pas de pauses imprévisibles, disposition des données en mémoire contrôlée (essentiel pour un ECS) |
| **Sûreté mémoire** | 2 | Éviter les plantages et failles dus aux pointeurs invalides |
| **Vitesse de développement** | 2 | Compilation, itération, lisibilité des erreurs |
| **Connaissance de l'équipe** | 2 | Le langage que l'équipe pratique le plus |

---

## Les données

### Vitesse et énergie : l'étude de Pereira et al.

L'étude la plus citée sur le sujet compare 27 langages sur les mêmes problèmes (ceux du *Computer Language Benchmarks Game*), en mesurant l'énergie consommée, le temps d'exécution et la mémoire ([Pereira et al., SLE 2017](https://greenlab.di.uminho.pt/wp-content/uploads/2017/10/sleFinal.pdf), [DOI](https://doi.org/10.1145/3136014.3136031)). Extrait du tableau 4, résultats normalisés (1,00 = le meilleur ; 2,00 = deux fois plus) :

| Langage | Énergie | Temps | Mémoire |
|---|---|---|---|
| C | **1,00** | **1,00** | 1,17 |
| Rust | 1,03 | 1,04 | 1,54 |
| **C++** | **1,34** | **1,56** | **1,34** |
| Java | 1,98 | 1,89 | 6,01 |
| C# | 3,14 | 3,14 | 2,85 |
| Go | 3,23 | 2,83 | 1,05 |
| Python | **75,88** | **71,90** | 2,80 |

Ce qu'il faut en retenir :

- **C, Rust et C++ forment le groupe de tête.** Les écarts entre eux (de 1 à 1,5) dépendent surtout de la qualité de chaque programme de test, pas du langage lui-même.
- **Python est environ 70 fois plus lent que le C**, et environ 45 fois plus lent que le C++ (71,90 / 1,56), sur ces problèmes de calcul. À l'échelle d'un jeu, une tâche qui prend 0,2 ms en C++ en prendrait environ 9 en Python, soit plus de la moitié du budget d'une frame.
- **Limites :** les auteurs reconnaissent que de meilleurs programmes et de meilleurs compilateurs peuvent changer le classement. Une étude de 2024 conteste la méthode : selon ses auteurs, une fois le temps d'exécution pris en compte, le choix du langage n'a pas d'effet significatif sur l'énergie, et les écarts viennent surtout des implémentations (parallélisme, ramasse-miettes, compilation à la volée) ([van Kempen et al., *It's Not Easy Being Green*, 2024](https://arxiv.org/html/2410.05460v3)). L'équipe du Benchmarks Game rappelle aussi que le chiffre « C++ consomme 56 % de plus que C », souvent repris, vient en réalité de la colonne **temps** ([Benchmarks Game](https://benchmarksgame-team.pages.debian.net/benchmarksgame/energy-efficiency.html)). On retient donc des **ordres de grandeur** : compilé natif ≈ 1×, machine virtuelle ≈ 2-3×, interprété ≈ 20-80×.

### Les pauses du ramasse-miettes

C#, Java, Go et Python libèrent la mémoire avec un **ramasse-miettes** (*garbage collector*), qui s'exécute quand il le décide. La documentation de Unity, moteur écrit en C++ mais scripté en C#, l'explique : ces interruptions produisent des « *GC spikes* » qui « *interrompent la cadence régulière de 60 fps* ». Unity propose un mode incrémental qui ne rend pas le ramassage plus rapide, mais l'étale sur plusieurs frames ([Unity, *Incremental garbage collection*](https://docs.unity3d.com/Manual/performance-incremental-garbage-collection.html)). En C++, la mémoire est libérée à un moment connu (fin de portée, `destroyEntity`), sans pause cachée.

### Le prix du C++ : la sûreté mémoire

L'équipe sécurité de Chromium (navigateur écrit en C++) indique qu'« *environ 70 % de [leurs] bugs de sécurité graves sont des problèmes de sûreté mémoire* », c'est-à-dire des erreurs de pointeurs C/C++ ([Chromium, *Memory safety*](https://www.chromium.org/Home/chromium-security/memory-safety/)). C'est le principal argument de Rust. Le risque est réel, mais il se réduit avec de la discipline et des outils (voir [Conséquences](#décision-et-conséquences)).

### Le C++ dans l'industrie du jeu

- **Godot** est écrit en C++ ([Godot](https://github.com/godotengine/godot)), et le gameplay d'**Unreal Engine** se programme en C++ ([Epic Games, *Programming with C++*](https://dev.epicgames.com/documentation/en-us/unreal-engine/programming-with-cplusplus-in-unreal-engine)).
- **Minecraft (Bedrock Edition)** utilise EnTT, un ECS C++ ([EnTT](https://github.com/skypjack/entt)).
- Toutes les bibliothèques retenues par le projet (raylib et raylib-cpp, Asio, plf::colony) s'utilisent directement en C++, sans couche de traduction.

### Les versions du C++

Selon l'enquête JetBrains 2025 sur 1 800 développeurs C++, les standards les plus utilisés sont **C++20 (43 %)** et **C++23 (34 %)**, le C++26 étant à 7 % ([JetBrains, *The State of C++ 2025*](https://lp.jetbrains.com/the-state-of-cpp-2025/)). Ce sont les valeurs du tableau du rapport : son texte cite des chiffres décalés d'une ligne. Le projet compile en **C++20** (`CMAKE_CXX_STANDARD 20`), un standard complet et bien supporté par GCC, Clang et MSVC. Le Poc passe en C++23 sous GCC 14, uniquement pour `std::stacktrace` dans ses rapports d'erreur.

---

## Les options étudiées

### C++ (retenu)

**Points forts**

- **Rapide**, sans ramasse-miettes, avec un contrôle total de la disposition des données : indispensable pour un ECS dont tout l'intérêt est de ranger les composants de façon contiguë ([étude 04](04-architecture-ecs.md)).
- **Templates :** l'ECS est typé à la compilation. `View<Position, Velocity>`, la vérification des doublons par `static_assert` ou la déduction des types dans `createSystem` n'ont aucun coût à l'exécution.
- **RAII :** une `Texture2D` ou une `Window` libère sa ressource raylib dans son destructeur, sans appel manuel à oublier.
- **L'écosystème jeu le plus riche.**

**Points faibles**

- **Sûreté mémoire** non garantie par le langage.
- **Compilation lente** et messages d'erreur de templates difficiles à lire.

### C

**Points forts**

- Le plus rapide du tableau, et raylib est écrit en C.

**Points faibles**

- **Ni templates ni RAII :** un ECS générique en C passe par des `void *` et des macros. Le compilateur ne vérifie plus que l'on lit bien un `Position` là où l'on attend un `Position`.
- **Moins d'abstractions sûres** : chaque ressource (texture, socket) se libère à la main, sans destructeur.

### Rust

**Points forts**

- **Aussi rapide que le C** (1,03 en énergie), avec une **sûreté mémoire garantie** à la compilation.
- Un moteur ECS moderne existe : Bevy.

**Points faibles**

- **Le « borrow checker » s'accorde mal avec les accès croisés d'un jeu** (un système qui lit une entité pendant qu'il en modifie une autre). Le studio LogLog Games, après trois ans et plus de 100 000 lignes de Rust, l'a quitté en citant la lenteur d'itération, les refactorisations imposées et un écosystème jeu qui « *vit de promesses* » ([LogLog Games, *Leaving Rust gamedev after 3 years*, 2024](https://loglog.games/blog/leaving-rust-gamedev/)). C'est un témoignage, pas une mesure, mais il est représentatif d'un débat réel.
- **Peu pratiqué par l'équipe**, et les bibliothèques retenues (raylib, Asio) s'y utilisent à travers des interfaces non officielles.

### Python

**Points forts**

- **Le plus rapide à écrire**, et pygame permet de prototyper rapidement.

**Points faibles**

- **Environ 45 fois plus lent que le C++** sur du calcul pur. Pour un serveur qui simule des centaines d'entités et répond à plusieurs clients 60 fois par seconde, c'est éliminatoire.
- **Le GIL** (verrou global de l'interpréteur) empêche d'exécuter du code Python sur plusieurs cœurs à la fois. Une version sans GIL existe depuis Python 3.13, mais elle reste optionnelle : le GIL « *reste le comportement par défaut* » ([PEP 703](https://peps.python.org/pep-0703/)).
- **Distribution plus lourde** : il faut livrer l'interpréteur avec le jeu.

### C# et Java

**Points forts**

- Productifs, sûrs, avec de bons outils. C# dispose de Unity et MonoGame.

**Points faibles**

- **Ramasse-miettes** (voir ci-dessus) et 2 à 3 fois plus lents en moyenne dans l'étude de Pereira.
- **Les bibliothèques retenues sont en C ou en C++** : il faudrait passer par des interfaces de liaison, avec leur propre maintenance.

---

## Grille de comparaison

| Critère (poids) | C++ | C | Rust | Python | C# | Java |
|---|---|---|---|---|---|---|
| Performance (3) | 5 | 5 | 5 | 1 | 3 | 3 |
| Écosystème jeu (3) | 5 | 3 | 3 | 3 | 4 | 2 |
| Maîtrise de la mémoire (2) | 5 | 5 | 5 | 1 | 2 | 2 |
| Sûreté mémoire (2) | 2 | 1 | 5 | 4 | 4 | 4 |
| Vitesse de développement (2) | 3 | 2 | 2 | 5 | 4 | 4 |
| Connaissance de l'équipe (2) | 5 | 5 | 2 | 4 | 3 | 3 |
| **Total pondéré /100** | **86** | **71** | **74** | **57** | **67** | **59** |

---

## Décision et conséquences

**Le projet est écrit en C++20.** Pour compenser la faiblesse principale du langage, la sûreté mémoire, le projet s'impose déjà plusieurs garde-fous :

- **RAII partout :** les ressources (fenêtre, textures, sons) sont libérées par des destructeurs, et la fenêtre raylib est tenue par un `std::unique_ptr`.
- **Des handles plutôt que des pointeurs :** une `Entity` est un couple *(index, génération)*. Un handle périmé est détecté par `hasEntity` au lieu de désigner une autre entité ([notes de conception](../../ECS_DESIGN.fr.md#entity-et-entitymanager)).
- **Des références stables :** les composants ne changent jamais d'adresse tant qu'ils existent ([étude 07](07-stockage-des-composants.md)).
- **Les sanitizers :** le Poc est compilé par défaut avec AddressSanitizer et UndefinedBehaviorSanitizer (`POC_SANITIZE`), qui détectent à l'exécution les accès mémoire invalides et les comportements indéfinis.
- **L'analyse statique :** `clang-tidy` (`script/lint.sh`) tourne dans l'intégration continue.

**Ce que l'on accepte :** des temps de compilation plus longs et des erreurs de templates parfois obscures. Pour les limiter, l'API publique (`GameManager`) reste simple et ses erreurs de type sont documentées.

> [!NOTE]
> Le README annonce un badge « C++26 » et dit que GCC 14 est « requis pour le dialecte C++26 », alors que les `CMakeLists.txt` demandent C++20. Il faudra harmoniser les deux.

---

## Quand revoir ce choix

- **Pour un outil annexe** (éditeur de niveaux, script de génération d'assets, bot de test réseau), Python ou C# peuvent être plus productifs : le C++ n'est indispensable que pour le moteur, le client et le serveur.
- **Si les bugs mémoire deviennent la première source de plantages** malgré les sanitizers, il faudra les activer aussi sur le client et le serveur en développement.
- **Quand GCC, Clang et MSVC supporteront complètement le C++23**, le passage apportera `std::expected`, `std::print` et `std::stacktrace` à tout le projet.

---

## Sources

- Pereira et al., *Energy Efficiency across Programming Languages*, SLE 2017 : <https://greenlab.di.uminho.pt/wp-content/uploads/2017/10/sleFinal.pdf> (DOI : <https://doi.org/10.1145/3136014.3136031>)
- van Kempen, Kwon, Nguyen et Berger, *It's Not Easy Being Green: On the Energy Efficiency of Programming Languages*, 2024 : <https://arxiv.org/html/2410.05460v3>
- The Computer Language Benchmarks Game, à propos de l'efficacité énergétique : <https://benchmarksgame-team.pages.debian.net/benchmarksgame/energy-efficiency.html>
- Unity, *Incremental garbage collection* : <https://docs.unity3d.com/Manual/performance-incremental-garbage-collection.html>
- Chromium, *Memory safety* : <https://www.chromium.org/Home/chromium-security/memory-safety/>
- LogLog Games, *Leaving Rust gamedev after 3 years*, avril 2024 : <https://loglog.games/blog/leaving-rust-gamedev/>
- PEP 703, *Making the Global Interpreter Lock Optional in CPython* : <https://peps.python.org/pep-0703/>
- JetBrains, *The State of C++ 2025* : <https://lp.jetbrains.com/the-state-of-cpp-2025/>
- EnTT : <https://github.com/skypjack/entt>
- Godot : <https://github.com/godotengine/godot>
- Epic Games, *Programming with C++ in Unreal Engine* : <https://dev.epicgames.com/documentation/en-us/unreal-engine/programming-with-cplusplus-in-unreal-engine>
