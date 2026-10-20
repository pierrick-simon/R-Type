# 08 · Sparse set ou archétypes ?

> 🌐 **Langue :** Français · [English](../en/08-ecs-storage.md)

> **Décision :** l'ECS range ses composants en **sparse set** : un tableau par type de composant.
> **Statut :** adopté · **Date :** octobre 2026 · [Retour au sommaire](README.md)

## En bref

Il existe deux grandes façons de ranger les composants d'un ECS. Le **sparse set** (EnTT) utilise un tableau par type de composant. Les **archétypes** (Unity, Bevy, flecs) utilisent une table par combinaison de composants. Les archétypes parcourent plus vite les systèmes qui demandent beaucoup de composants, mais **chaque ajout ou retrait de composant déplace toute l'entité** d'une table à une autre. Un shoot'em up crée, détruit et marque des entités en permanence, et ses systèmes demandent rarement plus de trois composants : le sparse set y est plus adapté. Il est aussi beaucoup plus simple à écrire, et compatible avec des références stables.

---

## Contexte

L'[étude 04](04-architecture-ecs.md) justifie le choix d'un ECS, et l'[étude 07](07-stockage-des-composants.md) celui du conteneur (`plf::colony`). Celle-ci répond à la question intermédiaire : **comment organiser les composants entre eux ?**

Ce que fait le jeu, frame après frame :

- **beaucoup de changements de structure** : des balles et des explosions apparaissent et disparaissent à chaque frame, des ennemis reçoivent ou perdent des tags (`Stunned`, `DestroyOffScreen`…), le pod « Force » s'accroche et se détache ;
- **des systèmes sur peu de composants** : `Position + Velocity`, `Sprite + Texture + Position`, rarement plus de trois ;
- **peu d'entités** : quelques centaines à quelques milliers à l'écran, pas des millions.

---

## Critères

| Critère | Poids | Pourquoi il compte |
|---|---|---|
| **Ajout et retrait de composants** | 3 | Créations, destructions et tags à chaque frame |
| **Simplicité d'implémentation** | 3 | L'ECS est écrit et maintenu par l'équipe |
| **Parcours sur plusieurs composants** | 2 | C'est le travail des systèmes à chaque frame |
| **Stabilité des références** | 2 | Garantie offerte par le guide de l'ECS ([étude 07](07-stockage-des-composants.md)) |
| **Mémoire et fragmentation** | 1 | Peu d'entités : secondaire |

---

## Les options étudiées

### Sparse set simple (retenu)

Un tableau par type de composant. Pour un système sur plusieurs composants, on parcourt le plus petit tableau, puis on vérifie que l'entité a bien les autres composants.

**Points forts**

- **Ajouter ou retirer un composant ne touche qu'un seul tableau.** Les autres composants de l'entité ne bougent pas.
- **Simple :** un `ComponentArray` par type, et une vue qui fait la jointure.
- **Compatible avec des références stables**, puisqu'un composant n'a jamais besoin d'être déplacé ([étude 07](07-stockage-des-composants.md)).

**Points faibles**

- **La jointure coûte une recherche par entité** et par composant demandé, en plus du parcours.

### Sparse set avec groupes (EnTT)

EnTT peut regrouper certaines combinaisons de composants : les entités qui ont tous les composants d'un groupe sont rangées au début de chaque tableau, dans le même ordre.

**Points forts**

- **Le parcours d'un groupe est aussi rapide qu'un tableau unique.** C'est la meilleure vitesse de parcours possible ([skypjack, *ECS back and forth, part 2*](https://skypjack.github.io/2019-03-07-ecs-baf-part-2/)).

**Points faibles**

- **Un seul ordre possible par tableau :** un type de composant ne peut appartenir qu'à un seul groupe.
- **Les éléments sont réordonnés** à chaque entrée ou sortie du groupe, donc ils changent d'adresse.
- **Plus complexe**, et il faut choisir les groupes à la main selon l'usage.

### Archétypes (Unity, Bevy, flecs)

Une table par combinaison exacte de composants : toutes les entités qui ont exactement `Position + Velocity + Sprite` sont dans la même table, colonne par colonne.

**Points forts**

- **Parcours très rapide** : un système ne visite que les tables qui l'intéressent, sans aucune vérification par entité.

**Points faibles**

- **Chaque ajout ou retrait de composant déplace l'entité** et tous ses composants vers une autre table. Le créateur d'EnTT l'écrit ainsi : « *chaque fois qu'un composant est ajouté ou retiré, une entité et tous ses composants sont déplacés d'un archétype à un autre* ». Unity le confirme dans sa documentation : « *déplacer fréquemment des entités est coûteux et réduit les performances* » ([Unity Entities](https://docs.unity3d.com/Packages/com.unity.entities@1.0/manual/concepts-archetypes.html)).
- **Fragmentation :** « *le nombre d'archétypes peut exploser* » quand les combinaisons de composants sont nombreuses ([skypjack](https://skypjack.github.io/2019-03-07-ecs-baf-part-2/)). Chaque tag ajouté crée une nouvelle table.
- **Les composants changent d'adresse** à chaque déplacement : pas de références stables.
- **Complexe à écrire :** gestion des tables, des blocs mémoire (16 Kio chez Unity) et du graphe de transitions entre archétypes.

### Hybride (Bevy)

Bevy laisse choisir le stockage **composant par composant** : en table (archétype) par défaut, ou en sparse set. Sa documentation résume le compromis : le stockage en table est « *optimisé pour un parcours rapide* », le sparse set « *pour ajouter et retirer rapidement un composant* », au prix de requêtes plus lentes ([Bevy Cheat Book](https://bevy-cheatbook.github.io/patterns/component-storage.html)).

**Points forts**

- Le meilleur des deux, au choix.

**Points faibles**

- **Deux stockages à écrire et à maintenir**, et un choix de plus pour chaque composant.

---

## Les données

### Le benchmark `ecs_benchmark`

Le projet [ecs_benchmark](https://github.com/abeimler/ecs_benchmark) compare plusieurs bibliothèques ECS C++ sur les mêmes opérations. Extrait pour **EnTT** (sparse set) et **flecs** (archétypes), Linux, GCC 14.2, EnTT 3.13.2, flecs 4.0.1 :

| Opération | Entités | EnTT (sparse set) | flecs (archétypes) |
|---|---|---|---|
| Retirer puis rajouter un composant | ≈ 65 000 | **1 ms** | 15 ms |
| Retirer puis rajouter un composant | ≈ 1 million | **26 ms** | 250 ms |
| Créer des entités (2 composants) | ≈ 1 million | **68 ms** | 409 ms |
| Détruire des entités (2 composants) | ≈ 1 million | **43 ms** | 60 ms |
| Mettre à jour 2 systèmes | ≈ 1 million | 12 ms | 12 ms |
| Mettre à jour 7 systèmes, entités variées | ≈ 1 million | 40 ms | **19 ms** |

Ce qu'on en retient :

- **Pour ajouter et retirer des composants, le sparse set est environ 10 fois plus rapide** (26 ms contre 250 ms).
- **Pour créer des entités, il est environ 6 fois plus rapide.**
- **Les archétypes ne prennent l'avantage que sur les parcours complexes** (7 systèmes sur des entités variées), avec un facteur 2.
- **Limites :** ces chiffres comparent deux bibliothèques complètes, pas seulement leur mode de stockage. Les résultats ne sont pas datés précisément, et ils portent sur un million d'entités, bien plus qu'un R-Type. On n'en garde que les **rapports** entre les deux approches.

### Ce que cela donne pour un R-Type

| Besoin du jeu | Fréquence | Approche favorisée |
|---|---|---|
| Créer et détruire des balles, des explosions | Plusieurs fois par frame | Sparse set (≈ 6 à 10× plus rapide) |
| Ajouter et retirer des tags | Fréquent | Sparse set (pas de déplacement d'entité) |
| Systèmes sur 1 à 3 composants | Chaque frame | Égalité (2 systèmes : 12 ms contre 12 ms) |
| Systèmes sur beaucoup de composants | Rare | Archétypes |

---

## Grille de comparaison

| Critère (poids) | Sparse set simple | Sparse set + groupes | Archétypes | Hybride |
|---|---|---|---|---|
| Ajout et retrait de composants (3) | 5 | 4 | 2 | 4 |
| Simplicité d'implémentation (3) | 5 | 2 | 2 | 1 |
| Parcours sur plusieurs composants (2) | 3 | 5 | 5 | 4 |
| Stabilité des références (2) | 5 | 2 | 1 | 3 |
| Mémoire et fragmentation (1) | 4 | 4 | 3 | 3 |
| **Total pondéré /100** | **91** | **65** | **49** | **58** |

---

## Décision et conséquences

**L'ECS utilise un sparse set simple** : un `ComponentArray<T>` par type (`dense` dans un `plf::colony`, `sparse` dans un `std::vector`), et une vue qui parcourt le plus petit tableau puis vérifie les autres ([notes de conception](../../ECS_DESIGN.fr.md#view)).

**Ce que l'on accepte :** une recherche dans `sparse` par entité et par composant supplémentaire, à chaque parcours. C'est un accès direct par index (O(1)), négligeable aux échelles du jeu.

**Ce qui guide l'écriture du jeu :**

- **Les tags ne coûtent presque rien** : on peut en ajouter et en retirer librement pour marquer des états.
- **Mettre en premier dans la vue le composant le plus rare n'est pas nécessaire** : la vue choisit d'elle-même le plus petit tableau.

---

## Quand revoir ce choix

- **Un système parcourt des dizaines de milliers d'entités sur 4 composants ou plus**, et le profilage montre que la jointure domine : ajouter des **groupes** sur cette combinaison précise, à la manière d'EnTT, plutôt que de passer aux archétypes.
- **Le jeu n'ajoute et ne retire presque jamais de composants** (entités figées après leur création) : les archétypes deviendraient intéressants, mais au prix des références stables.

---

## Sources

- Michele Caini (skypjack), *ECS back and forth, part 2: Where are my entities?*, mars 2019 : <https://skypjack.github.io/2019-03-07-ecs-baf-part-2/>
- Sander Mertens, *Entity Component System FAQ* : <https://github.com/SanderMertens/ecs-faq>
- Unity, *Entities: Archetypes concepts* : <https://docs.unity3d.com/Packages/com.unity.entities@1.0/manual/concepts-archetypes.html>
- Bevy Cheat Book, *Component Storage (Table/Sparse-Set)* : <https://bevy-cheatbook.github.io/patterns/component-storage.html>
- abeimler, *ecs_benchmark* : <https://github.com/abeimler/ecs_benchmark>
