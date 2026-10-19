# Études comparatives

> 🌐 **Langue :** Français · [English](../en/README.md)

Ce dossier rassemble les **études comparatives** des choix techniques du projet R-Type. Chaque étude répond à une question du type « pourquoi X plutôt que Y ? » : elle compare les options possibles selon des critères explicites, s'appuie sur des chiffres sourcés, puis présente la décision retenue, son coût et les conditions qui pourraient la remettre en cause.

- **Public visé :** l'équipe, et toute personne qui rejoint le projet ou s'intéresse à ses choix techniques.
- **Pour utiliser le moteur**, consultez plutôt [le guide de création d'un jeu](../../GAME_GUIDE.fr.md) et [le guide de l'ECS](../../ECS_GUIDE.fr.md).

---

## Les études

| N° | Question | Choix retenu | Principales alternatives |
|---|---|---|---|
| 01 | [Pourquoi Windows et Linux ?](01-systemes-exploitation.md) | Windows + Linux | macOS, consoles, mobile, Linux seul |
| 02 | [Pourquoi le C++ ?](02-langage.md) | C++20 | C, Rust, Python, C#, Java |
| 03 | [Pourquoi CMake ?](03-systeme-de-build.md) | CMake + `FetchContent` | Make, Meson, Bazel, Premake, projets Visual Studio |
| 04 | [Pourquoi un ECS ?](04-architecture-ecs.md) | ECS maison (sparse set) | Héritage objet, composition objet (type Unity), ECS existant (EnTT, flecs) |
| 05 | [Pourquoi raylib ?](05-bibliotheque-graphique.md) | raylib 5.5 + raylib-cpp | SFML, SDL3, GLFW + OpenGL, moteur complet |
| 06 | [Pourquoi Asio pour le réseau ?](06-bibliotheque-reseau.md) | Asio autonome (sans Boost) | Sockets natifs, Boost.Asio, ENet, GameNetworkingSockets, SFML Network |
| 07 | [Pourquoi `plf::colony` pour stocker les composants ?](07-stockage-des-composants.md) | `plf::colony` | `std::vector`, `std::deque`, `std::list`, `std::unordered_map`, `std::vector<std::unique_ptr>` |
| 08 | [Sparse set ou archétypes ?](08-stockage-ecs.md) | Sparse set simple | Sparse set avec groupes (EnTT), archétypes (Unity, Bevy, flecs), hybride |
| 09 | [Comment récupérer les dépendances ?](09-gestion-des-dependances.md) | `FetchContent` | CPM, vcpkg, Conan, paquets système, sources copiées, sous-modules |
| 10 | [UDP ou TCP, avec Asio ?](10-protocole-reseau.md) | UDP, avec une connexion maison | TCP seul, TCP et UDP ensemble |
| 11 | [Quel framework de tests ?](11-framework-de-tests.md) | Criterion | GoogleTest, Catch2, doctest, Boost.Test, lanceur maison |

## La méthode

Chaque étude suit le même plan, inspiré des *Architecture Decision Records* (ADR), un format proposé par Michael Nygard pour documenter une décision avec son contexte et ses conséquences ([source](https://cognitect.com/blog/2011/11/15/documenting-architecture-decisions)) :

1. **En bref :** la décision et sa raison principale, en trois lignes.
2. **Contexte :** le problème à résoudre et les contraintes du projet.
3. **Critères :** ce qui compte pour ce choix, avec un poids de 1 (secondaire) à 3 (essentiel).
4. **Options étudiées :** chaque alternative sérieuse, avec ses forces et ses faiblesses.
5. **Données :** benchmarks, enquêtes et documentation officielle, toujours sourcés.
6. **Grille de comparaison :** une note de 1 à 5 par critère et par option, puis un total pondéré.
7. **Décision et conséquences :** ce qui a été choisi, ce que l'on gagne, ce que l'on accepte de perdre.
8. **Quand revoir ce choix :** les signaux qui justifieraient de reconsidérer la décision.
9. **Sources.**

### Lire les grilles de comparaison

Les notes (1 = mauvais, 5 = excellent) sont **une appréciation de l'équipe**, pas une mesure. Chacune est justifiée dans le texte de l'étude. Le total pondéré se calcule ainsi : somme des (note × poids), ramenée sur 100. Il aide à résumer, mais ne remplace pas la lecture : un seul critère éliminatoire (par exemple « ne fonctionne pas sous Windows ») peut suffire à écarter une option.

### Lire les chiffres

Un benchmark mesure un cas précis, sur une machine précise. Les études indiquent toujours **ce qui a été mesuré**, **par qui** et **dans quelles conditions**, et en tirent des ordres de grandeur plutôt que des vérités absolues. Quand une source est contestée (c'est le cas du classement énergétique des langages, par exemple), l'étude le signale.

---

## Les contraintes communes à toutes les études

Quelques besoins du projet reviennent dans toutes les études :

- **Un jeu multijoueur en temps réel :** un serveur qui fait autorité sur la partie, et des clients qui affichent le jeu. À 60 images par seconde, chaque frame dispose de **16,7 ms**, côté client comme serveur.
- **Deux systèmes cibles :** Windows et Linux ([étude 01](01-systemes-exploitation.md)).
- **Un moteur maison et réutilisable :** le dossier `Engine/` (ECS, affichage, réseau) est indépendant du jeu, et le dossier `Games/` peut accueillir plusieurs jeux. Le projet maîtrise donc sa boucle de jeu, son modèle d'objets et son réseau, plutôt que de les déléguer à un moteur existant.
- **Une petite équipe :** chaque outil doit pouvoir être pris en main rapidement, et le projet doit se construire sans installation compliquée.

> [!NOTE]
> Ces études reflètent l'état du dépôt au 19 octobre 2026. Le réseau (`engine-network`) est implémenté sur la branche `feature/network`, pas encore fusionnée dans `dev` : les études 06 et 10 décrivent cette implémentation.
