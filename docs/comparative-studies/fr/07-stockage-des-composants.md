# 07 · Pourquoi `plf::colony` pour stocker les composants ?

> 🌐 **Langue :** Français · [English](../en/07-component-storage.md)

> **Décision :** dans l'ECS, les composants d'un même type sont stockés dans un **`plf::colony`**, l'implémentation de référence du futur `std::hive` (C++26).
> **Statut :** adopté · **Date :** octobre 2026 · [Retour au sommaire](README.md)

## En bref

Un `std::vector` est le conteneur le plus rapide à parcourir. Mais pour supprimer un élément sans laisser de trou, il **déplace** un autre élément, ce qui invalide toute référence vers lui. `plf::colony` garantit qu'**un composant ne change jamais d'adresse tant qu'il existe**, avec des insertions et suppressions en O(1). C'est cette garantie qui permet au code de jeu de garder une référence vers un composant, à `getVector()` de rester valide même quand des entités sont détruites, et à l'ECS de rester simple. Le prix est un parcours **environ 8 fois plus lent** qu'un `vector` dans le benchmark de référence (Daniel Lemire, 2026). Aux échelles d'un R-Type, cela reste de l'ordre de **1 % du budget d'une frame**.

---

## Contexte

Dans l'ECS, chaque type de composant a son `ComponentArray`, organisé en *sparse set* ([notes de conception](../../ECS_DESIGN.fr.md#icomponentarray-et-componentarray)) :

- **`dense`** contient les composants eux-mêmes, que les systèmes parcourent à chaque frame ;
- **`sparse`** fait le lien entre une entité et son composant dans `dense`, en O(1).

Le choix du conteneur de `dense` décide de quatre choses :

1. **la vitesse des systèmes**, qui parcourent `dense` à chaque frame ;
2. **la vitesse des créations et destructions**, très fréquentes dans un shoot'em up (des balles apparaissent et disparaissent à chaque frame) ;
3. **la validité des références** : le code de jeu peut-il garder un `Position &` d'une frame à l'autre ? Et ce que renvoie `getVector()` reste-t-il valide si une entité est détruite pendant la boucle ?
4. **la simplicité de l'ECS** : que doit contenir `sparse` pour retrouver un composant ?

---

## Critères

| Critère | Poids | Pourquoi il compte |
|---|---|---|
| **Stabilité des références** | 3 | Garantie offerte par le guide de l'ECS, et sur laquelle reposent `getVector()` et les références conservées par le jeu |
| **Insertion et suppression** | 3 | Des dizaines de créations et destructions par frame |
| **Vitesse de parcours** | 2 | Chaque système parcourt `dense` à chaque frame |
| **Simplicité de l'ECS** | 2 | Moins de code de synchronisation entre `sparse` et `dense`, c'est moins de bugs |
| **Mémoire** | 1 | Quelques milliers d'entités : secondaire |
| **Pérennité** | 1 | Un conteneur standard ou en voie de l'être |

---

## Les options étudiées

### `std::vector` avec « swap-and-pop » (le choix d'EnTT)

Pour supprimer un élément, on le remplace par le dernier, puis on retire le dernier.

**Points forts**

- **Le parcours le plus rapide possible :** les éléments sont contigus, sans trou.
- **Suppression en O(1).**

**Points faibles**

- **Le dernier élément change d'adresse** à chaque suppression. Toute référence vers lui devient invalide, et `sparse` doit être corrigé. Garder un `Position &` d'une frame à l'autre devient impossible, et `getVector()` serait invalidé par la moindre destruction.
- **Une insertion peut réallouer tout le tableau** et invalider *toutes* les références d'un coup.

EnTT, qui suit ce modèle, propose d'ailleurs une option de « *stabilité des pointeurs* » pour les composants qui en ont besoin ([EnTT](https://github.com/skypjack/entt)). C'est bien la preuve que ce besoin existe.

### `std::vector` avec des trous (*tombstones*)

On marque l'élément comme supprimé sans le déplacer, et on réutilise sa case plus tard.

**Points forts**

- Les éléments ne bougent pas lors d'une suppression.

**Points faibles**

- **Une insertion qui dépasse la capacité réalloue tout** : la stabilité n'est pas garantie, sauf à fixer une taille maximale à l'avance.
- Le parcours doit tester chaque case, et les trous s'accumulent.

### `std::deque`

**Points forts**

- Un ajout en fin ne déplace pas les éléments existants.

**Points faibles**

- **Une suppression au milieu déplace des éléments.** On retombe sur les trous de l'option précédente.

### `std::list`

**Points forts**

- **Stable** : un nœud ne bouge jamais.

**Points faibles**

- **Une allocation par élément** : environ 14 ns par insertion, contre 0,8 pour un `vector` (voir [Données](#les-données)).
- **32 octets de surcoût** par élément pour deux pointeurs et l'allocation. Après quelques suppressions et insertions, les nœuds sont éparpillés en mémoire, et le parcours devient lent.

### `std::unordered_map<Entity, Component>` ou `std::vector<std::unique_ptr<Component>>`

**Points forts**

- **Stables** (chaque élément est alloué à part).

**Points faibles**

- **Éparpillés en mémoire** : on perd tout l'intérêt d'un ECS, qui est de ranger les données côte à côte ([étude 04](04-architecture-ecs.md#le-coût-du-cache)).

### `plf::colony` (retenu)

Une chaîne de blocs mémoire de taille croissante. Un élément supprimé est marqué dans un *skipfield*, que le parcours saute en O(1), et sa case est réutilisée par une insertion suivante. Un bloc vide est rendu au système ([plf::colony](https://plflib.org/colony.htm)).

**Points forts**

- **Stabilité garantie :** les pointeurs et itérateurs vers un élément restent valides tant que cet élément n'est pas lui-même supprimé, quelles que soient les insertions et suppressions ailleurs.
- **Insertion et suppression en O(1) amorti**, sans réallocation des éléments existants.
- **Un ECS plus simple :** comme un itérateur reste valide, `sparse` stocke **directement l'itérateur** vers le composant. Il n'y a rien à resynchroniser lors d'une suppression.
- **Bientôt standard :** `std::hive`, issu de colony, a été adopté dans le C++26 en février 2025 ([P3933R1](https://open-std.org/jtc1/sc22/wg21/docs/papers/2026/p3933r1.html), [cppreference](https://cppreference.net/cpp/header/hive.html)).

**Points faibles**

- **Parcours plus lent qu'un `vector`** : un saut dans le *skipfield* et un changement de bloc de temps en temps.
- **Pas d'index stable partagé entre deux conteneurs.** L'entité doit donc être rangée avec son composant (`ComponentPack`), soit au moins 16 octets de plus par composant ([notes de conception](../../ECS_DESIGN.fr.md#icomponentarray-et-componentarray)).
- **Une dépendance externe**, tant qu'aucune bibliothèque standard ne fournit `std::hive`.

---

## Les données

### 1. Daniel Lemire : `std::hive` contre `vector` et `list`

Daniel Lemire, chercheur reconnu en performance, a mesuré `plf::hive` (l'implémentation de `std::hive` par l'auteur de colony) sur des entiers de 64 bits, avec GCC 16.1, sur un Intel Xeon Gold ([Lemire, *How fast is C++26's std::hive?*, août 2026](https://lemire.me/blog/2026/08/02/how-fast-is-c26s-stdhive/)) :

| Opération (1 million d'éléments) | `vector` | `hive` | `list` |
|---|---|---|---|
| Insertion (ns/élément) | 0,81 | 1,57 | 14,22 |
| Parcours (ns/élément) | **0,22** | 1,77 | 1,51 |
| Suppression de la moitié, au hasard (ns/élément) | 3,0 | **2,1** | 77,4 |
| Mémoire (octets/élément) | 8,4 | 9,4 | 32,0 |

Sa conclusion : « ***std::hive n'est pas un vector plus rapide*** ». Il apporte des références stables et des suppressions peu coûteuses, pour une mémoire bien inférieure à celle d'une liste. Il note aussi qu'à 10 millions d'éléments, la suppression redevient plus rapide avec un `vector` (1,3 contre 2,5 ns).

### 2. L'auteur de colony : les cas de forte modification

Les benchmarks publiés par l'auteur de colony confirment que le `vector` a « *facilement* » les meilleures performances en parcours pur. En revanche, dans les tests de « *forte modification* » (5 % et 10 % d'éléments supprimés à chaque frame, ce qui ressemble aux balles d'un shoot'em up), **colony arrive en tête** ([plflib, benchmarks Haswell/GCC](https://plflib.org/benchmarks_haswell_gcc.htm)). Ce sont les mesures de l'auteur lui-même : elles vont dans le même sens que celles de Lemire, mais restent à prendre avec ce recul.

### 3. Ce que cela représente pour un R-Type

Une estimation à partir des chiffres de Lemire (environ **1,8 ns par élément** pour un parcours de `hive`) :

| Situation | Coût d'un parcours | Part d'une frame à 60 FPS (16,7 ms) |
|---|---|---|
| 1 système sur 1 000 entités | ≈ 2 µs | ≈ 0,01 % |
| 10 systèmes sur 1 000 entités | ≈ 18 µs | ≈ 0,1 % |
| 10 systèmes sur 10 000 entités | ≈ 0,18 ms | ≈ 1 % |

Ce sont des **ordres de grandeur**, pas des mesures du projet : les composants réels sont plus gros qu'un entier de 64 bits, et un système sur plusieurs composants fait en plus une recherche dans `sparse` pour chaque entité. Ils suffisent à montrer que le surcoût de `colony` ne pèse pas aux échelles d'un shoot'em up, où l'écran compte rarement plus de quelques milliers d'objets.

---

## Grille de comparaison

| Critère (poids) | `vector` swap-and-pop | `vector` + trous | `deque` | `list` | `unordered_map` | `vector<unique_ptr>` | **`colony`** |
|---|---|---|---|---|---|---|---|
| Stabilité des références (3) | 1 | 2 | 2 | 5 | 5 | 5 | 5 |
| Insertion et suppression (3) | 5 | 4 | 2 | 2 | 3 | 3 | 5 |
| Vitesse de parcours (2) | 5 | 4 | 4 | 2 | 1 | 2 | 3 |
| Simplicité de l'ECS (2) | 3 | 3 | 3 | 4 | 5 | 3 | 5 |
| Mémoire (1) | 5 | 3 | 4 | 1 | 1 | 2 | 4 |
| Pérennité (1) | 5 | 5 | 5 | 5 | 5 | 5 | 4 |
| **Total pondéré /100** | **73** | **67** | **58** | **65** | **70** | **68** | **90** |

---

## Décision et conséquences

**`dense` est un `plf::colony<ComponentPack>`**, et `sparse` un `std::vector` d'itérateurs vers `dense`. La version de plf::colony est figée sur un commit précis dans `Engine/CMakeLists.txt`.

**Ce que cela permet**, et que le [guide de l'ECS](../../ECS_GUIDE.fr.md) promet :

- **Garder une référence vers un composant** tant que ce composant existe ([guide, section 3](../../ECS_GUIDE.fr.md#les-références-vers-un-composant)).
- **Parcourir le résultat de `getVector()` en détruisant des entités**, même hors d'`update()` : les références des autres entités restent valides ([guide, section 4](../../ECS_GUIDE.fr.md#parcourir-une-vue-dans-un-système--getvector)).
- **Un `ComponentArray` court et sans code de resynchronisation.**

**Ce que l'on accepte :**

- **Un parcours environ 8 fois plus lent** qu'un `vector` compacté selon Lemire, soit de l'ordre de 1 % du budget d'une frame aux échelles d'un R-Type ([estimation](#3-ce-que-cela-représente-pour-un-r-type)).
- **16 octets de plus par composant**, pour ranger l'entité avec lui.
- **Une dépendance externe** tant que `std::hive` n'est pas livré. Selon Lemire, aucune bibliothèque standard ne le fournissait encore en août 2026.

---

## Quand revoir ce choix

- **Quand libstdc++ et MSVC fourniront `std::hive`** : remplacer `plf::colony` par `std::hive`. Le comportement est le même, puisque colony en est l'implémentation de référence, et une dépendance disparaît.
- **Si le profilage montre que les parcours dominent la frame** (dizaines de milliers d'entités, beaucoup de systèmes) : envisager un `vector` compacté **seulement pour les composants qui n'ont pas besoin de références stables**, comme le permet EnTT. Attention : c'est un changement de la garantie offerte au code de jeu, à documenter dans le guide.

---

## Sources

- plf::colony, documentation : <https://plflib.org/colony.htm>
- plf::colony, benchmarks Haswell/GCC : <https://plflib.org/benchmarks_haswell_gcc.htm>
- Daniel Lemire, *How fast is C++26's std::hive?*, août 2026 : <https://lemire.me/blog/2026/08/02/how-fast-is-c26s-stdhive/>
- WG21, P3933R1 *constexpr hive* (historique de l'adoption de `std::hive`) : <https://open-std.org/jtc1/sc22/wg21/docs/papers/2026/p3933r1.html>
- cppreference, en-tête `<hive>` : <https://cppreference.net/cpp/header/hive.html>
- EnTT : <https://github.com/skypjack/entt>
