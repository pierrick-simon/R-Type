# 04 · Pourquoi un ECS ?

> 🌐 **Langue :** Français · [English](../en/04-ecs-architecture.md)

> **Décision :** les objets du jeu sont gérés par un **Entity-Component-System (ECS) écrit par l'équipe**, de type *sparse set*.
> **Statut :** adopté · **Date :** octobre 2026 · [Retour au sommaire](README.md)

## En bref

Dans un R-Type, les objets combinent sans cesse les mêmes comportements : bouger, tirer, encaisser des dégâts, être piloté au clavier, suivre un autre objet. Une **hiérarchie de classes** se bloque vite sur ces combinaisons. Un **ECS** les assemble librement : une entité est un simple identifiant, ses composants sont des données, et des systèmes appliquent la logique à toutes les entités qui ont les bons composants. Les données d'un même type sont rangées côte à côte, ce qui ménage le cache du processeur, et elles s'envoient facilement sur le réseau. L'équipe écrit son propre ECS plutôt que d'utiliser EnTT : il est taillé pour les besoins du projet (références stables, modifications différées pendant une frame), sans API à apprendre ni dépendance au cœur du moteur. L'orienté objet reste utilisé, mais pour structurer le moteur.

---

## Contexte

Un R-Type, c'est un vaisseau joueur, plusieurs types d'ennemis, des balles, des bonus, des explosions, des décors qui défilent, et la fameuse **« Force »**, un pod qui s'accroche au vaisseau, bloque les balles, tire, et peut être détaché puis relancé. Il faut aussi que le serveur envoie l'état de tous ces objets aux clients, plusieurs dizaines de fois par seconde.

La question est : **comment représenter ces objets dans le code ?**

---

## Critères

| Critère | Poids | Pourquoi il compte |
|---|---|---|
| **Combiner les comportements** | 3 | Le gameplay évolue sans cesse : ajouter un comportement à un objet doit être simple |
| **Performance** | 2 | Des centaines d'objets mis à jour à chaque frame, côté client et serveur |
| **Adapté au réseau** | 2 | L'état du jeu doit pouvoir être copié et envoyé facilement |
| **Facilité de prise en main** | 2 | Toute l'équipe écrit du gameplay |
| **Débogage, lisibilité** | 1 | Comprendre ce que fait un objet à un instant donné |
| **Maîtrise et adaptation au projet** | 2 | L'équipe doit comprendre l'outil, le déboguer et le faire évoluer selon ses besoins |

---

## Les options étudiées

### Option A : une hiérarchie de classes

```text
GameObject (virtual update, virtual draw)
├── Ship
│   ├── PlayerShip
│   └── EnemyShip
│       └── ShootingEnemy
├── Projectile
└── PowerUp
```

**Points forts**

- **Intuitive :** c'est l'orienté objet tel qu'on l'apprend.

**Points faibles**

- **Les combinaisons font exploser la hiérarchie.** La « Force » est-elle un `Projectile` (elle est lancée et inflige des dégâts), un `Ship` (elle tire) ou un `PowerUp` (on la ramasse) ? Il faudrait un héritage multiple, avec le problème du losange, ou une copie du code. C'est le problème classique que décrit Robert Nystrom dans *Game Programming Patterns* ([chapitre *Component*](https://gameprogrammingpatterns.com/component.html)).
- **Les classes de base grossissent** : tout ce qui est partagé « remonte » dans `GameObject`, qui finit par tout savoir faire.
- **Objets éparpillés en mémoire** : chaque objet est alloué séparément et parcouru par pointeur, ce qui coûte un défaut de cache à chaque objet.

### Option B : la composition objet (le modèle « GameObject » de Unity)

Un `GameObject` contient une liste de composants qui ont chacun leur méthode virtuelle `update()`.

**Points forts**

- **Résout les combinaisons :** la « Force » a un `Mover`, un `Weapon` et un `Damage`.

**Points faibles**

- **La logique reste dans les composants**, et les composants qui doivent communiquer (une `Weapon` qui lit la position du `Mover`) se cherchent les uns les autres.
- **Toujours des objets éparpillés** et un appel virtuel par composant et par frame.

### Option C : un ECS (retenu)

```text
Entité   = un identifiant (index + génération)
Composant = des données pures : Position, Velocity, Health, Weapon, Player...
Système  = une fonction : « pour toute entité avec Position et Velocity, avancer »
```

**Points forts**

- **Les combinaisons sont gratuites.** La « Force », c'est `Position + Velocity + Hitbox + Weapon + AttachedTo`. Pour la détacher, il suffit de retirer `AttachedTo`. Aucune classe à créer.
- **Données contiguës :** tous les `Position` sont rangés ensemble. Un système qui les parcourt lit la mémoire dans l'ordre, ce qui est le cas le plus favorable pour le cache du processeur (voir [Données](#les-données)).
- **Adapté au réseau :** l'état d'une entité, ce sont ses composants, des structs de données sans pointeurs ni logique. Les copier dans un paquet est direct.
- **Séparation nette :** les données (composants) ne connaissent pas la logique (systèmes). On ajoute un comportement sans toucher au reste.

**Points faibles**

- **Une autre façon de penser :** « qu'a cette entité ? » au lieu de « qu'est cet objet ? ». Le [guide de l'ECS](../../ECS_GUIDE.fr.md) et ses pièges documentés sont là pour accompagner cette transition.
- **Logique dispersée :** le comportement d'un ennemi est réparti entre plusieurs systèmes. Il faut lire l'ordre des systèmes pour le comprendre.

### Option D : un ECS existant (EnTT, flecs)

**Points forts**

- **Matures, optimisés et éprouvés :** EnTT est utilisé par Minecraft (Bedrock Edition) ([EnTT](https://github.com/skypjack/entt)), flecs propose notamment des relations entre entités, un langage de requêtes et un explorateur ([flecs](https://github.com/SanderMertens/flecs)).

**Points faibles**

- **Une dépendance externe au cœur du moteur :** au moindre comportement inattendu, il faut plonger dans une grosse base de code, et ses garanties (validité des références, modifications pendant un parcours) ne sont pas celles que le projet veut offrir à son code de jeu.
- **Une API riche, donc longue à apprendre**, pour des fonctionnalités dont un R-Type n'a pas besoin.

L'ECS du projet reprend d'ailleurs les idées d'EnTT (*sparse set*, vues, handles générationnels) : le [document de conception](../../ECS_DESIGN.fr.md) détaille ce qui a été repris, et ce qui a été fait différemment.

---

## Les données

### Le coût du cache

Lire un octet en mémoire vive peut coûter **des centaines de cycles** processeur. Quand le processeur lit la mémoire, il charge toute une **ligne de cache** (64 octets en général) : si les données suivantes sont juste à côté, elles sont déjà là. Robert Nystrom a mesuré l'effet en remplaçant des objets parcourus par pointeur par des tableaux contigus de composants : **la boucle de mise à jour est devenue cinquante fois plus rapide**. Il précise que le gain varie selon les machines ([*Game Programming Patterns*, chapitre *Data Locality*](https://gameprogrammingpatterns.com/data-locality.html)).

Ce chiffre est un cas favorable. À l'échelle d'un R-Type (quelques centaines à quelques milliers d'entités), la performance n'est pas le premier argument de l'ECS. Ce sont la **souplesse** et l'**adéquation au réseau** qui comptent. La performance est un bonus qui laisse de la marge au serveur.

### Qui utilise un ECS ?

- **Overwatch** (Blizzard) : son architecture de gameplay et son netcode reposent sur un ECS, présenté à la GDC 2017. Selon la présentation, l'équipe s'en sert pour « *contenir la complexité* » tout en ajoutant de nouvelles fonctionnalités ([GDC Vault, *Overwatch Gameplay Architecture and Netcode*](https://gdcvault.com/play/1024001/-Overwatch-Gameplay-Architecture-and)).
- **Minecraft Bedrock** : utilise EnTT ([EnTT](https://github.com/skypjack/entt)).
- **Unity DOTS** et **Bevy** (Rust) : des moteurs construits autour d'un ECS ([ECS FAQ](https://github.com/SanderMertens/ecs-faq)).

### La critique

L'ECS n'est pas toujours le bon outil. Le studio LogLog Games, après trois ans de développement, estime que beaucoup de jeux n'en ont pas besoin, et que l'ECS est parfois adopté par effet de mode plus que par nécessité ([LogLog Games](https://loglog.games/blog/leaving-rust-gamedev/)). L'argument vaut pour un petit jeu solo au gameplay figé. Il pèse moins ici : le jeu est **multijoueur**, son état doit être **sérialisé**, et le moteur doit pouvoir accueillir plusieurs jeux.

### Sparse set ou archétypes ?

*Résumé ci-dessous ; la comparaison complète, avec des benchmarks, est dans l'[étude 08](08-stockage-ecs.md).*

Il existe deux grandes façons de stocker les composants ([ECS FAQ, Sander Mertens](https://github.com/SanderMertens/ecs-faq)) :

| | **Sparse set** (EnTT, ce projet) | **Archétypes** (Unity DOTS, Bevy, flecs) |
|---|---|---|
| **Principe** | Un tableau par type de composant | Une table par combinaison de composants |
| **Ajouter ou retirer un composant** | Rapide : un seul tableau modifié | Coûteux : l'entité change de table, toutes ses données sont déplacées |
| **Parcourir plusieurs composants** | On parcourt le plus petit tableau, et on cherche les autres | Très rapide : tout est aligné dans la même table |
| **Complexité d'implémentation** | Faible | Élevée |

Un R-Type ajoute et retire beaucoup de composants (balles créées et détruites à chaque frame, tags ajoutés et retirés), et ses systèmes portent rarement sur plus de trois composants. Le **sparse set** est donc le meilleur compromis, et de loin le plus simple à écrire.

---

## Grille de comparaison

| Critère (poids) | A. Héritage | B. Composition objet | C. ECS maison | D. EnTT / flecs |
|---|---|---|---|---|
| Combiner les comportements (3) | 1 | 4 | 5 | 5 |
| Performance (2) | 2 | 2 | 4 | 5 |
| Adapté au réseau (2) | 2 | 3 | 5 | 5 |
| Facilité de prise en main (2) | 5 | 4 | 3 | 2 |
| Débogage, lisibilité (1) | 4 | 4 | 3 | 3 |
| Maîtrise et adaptation au projet (2) | 4 | 4 | 5 | 2 |
| **Total pondéré /100** | **55** | **70** | **87** | **77** |

---

## Décision et conséquences

**Les objets du jeu sont gérés par un ECS maison, de type sparse set.** Son API tient dans une seule classe, `RType::GameManager`, documentée dans le [guide de l'ECS](../../ECS_GUIDE.fr.md).

**L'orienté objet n'est pas abandonné, il change de place.** Il structure le **moteur** : une façade (`GameManager`), des interfaces (`IComponentArray`, `ISystem`) et des classes templates qui les implémentent, de l'encapsulation (`EntityManager`, `ComponentManager` et `SystemManager` sont des détails internes). L'ECS organise les **données du jeu**. Les deux approches se complètent.

**Ce que l'on accepte :**

- **Un temps d'apprentissage**, compensé par le guide, ses pièges documentés et les plus de 200 tests du Poc.
- **La logique d'un objet est répartie entre plusieurs systèmes.** L'ordre des systèmes doit être documenté et réfléchi ([guide de création d'un jeu](../../GAME_GUIDE.fr.md#5-lordre-des-systèmes)).
- **Un moteur à maintenir :** chaque fonctionnalité (vues, systèmes par frame, `getVector()`) est écrite et testée par l'équipe.

---

## Quand revoir ce choix

- **Les systèmes portent souvent sur 4 composants ou plus, sur des milliers d'entités**, et le profilage montre que la jointure coûte cher : envisager des groupes (comme EnTT) ou des archétypes.
- **L'ECS devient le goulot de l'équipe** (fonctionnalités manquantes, bugs du moteur) : comparer le coût de maintenance avec une bibliothèque comme EnTT, et migrer si elle couvre les mêmes garanties.

---

## Sources

- Robert Nystrom, *Game Programming Patterns*, chapitre *Component* : <https://gameprogrammingpatterns.com/component.html>
- Robert Nystrom, *Game Programming Patterns*, chapitre *Data Locality* : <https://gameprogrammingpatterns.com/data-locality.html>
- Sander Mertens, *Entity Component System FAQ* : <https://github.com/SanderMertens/ecs-faq>
- Timothy Ford, *Overwatch Gameplay Architecture and Netcode*, GDC 2017 : <https://gdcvault.com/play/1024001/-Overwatch-Gameplay-Architecture-and>
- EnTT : <https://github.com/skypjack/entt>
- flecs : <https://github.com/SanderMertens/flecs>
- LogLog Games, *Leaving Rust gamedev after 3 years* : <https://loglog.games/blog/leaving-rust-gamedev/>
