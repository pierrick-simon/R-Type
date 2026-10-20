# 06 · Pourquoi Asio pour le réseau ?

> 🌐 **Langue :** Français · [English](../en/06-network-library.md)

> **Décision :** la bibliothèque réseau du moteur (`engine-network`) reposera sur **Asio, en version autonome** (sans Boost).
> **Statut :** adopté, implémenté sur la branche `feature/network` · **Date :** octobre 2026 · [Retour au sommaire](README.md)

## En bref

Asio offre **une seule API de sockets UDP et TCP pour Windows et Linux**, avec un **modèle asynchrone** qui permet à un serveur de servir plusieurs clients sans un thread par client. Sa version autonome **n'a besoin que du C++11 standard**, sans Boost. C'est la bibliothèque réseau C++ la plus éprouvée : elle a même servi de base au projet de réseau standard du C++ (le *Networking TS*). Les alternatives orientées jeu (ENet, GameNetworkingSockets) imposent leur propre protocole, alors que le projet veut concevoir le sien, adapté à son ECS.

---

## Contexte

Le R-Type est multijoueur : un **serveur** fait autorité sur la partie, et plusieurs **clients** lui envoient leurs entrées et reçoivent l'état du jeu, à travers Internet.

Sous Linux, les sockets sont ceux de BSD (`<sys/socket.h>`) ; sous Windows, c'est Winsock (`<winsock2.h>`), avec une initialisation, des types et des codes d'erreur différents. Les mécanismes d'attente sur plusieurs sockets diffèrent aussi : `epoll` sous Linux, *I/O Completion Ports* sous Windows.

**État actuel :** Asio est déjà intégré au build (`Engine/CMakeLists.txt`, version `asio-1-34-2`, avec `ASIO_STANDALONE` et `ASIO_NO_DEPRECATED`) et lié à `engine-network`. Sur `dev`, cette bibliothèque ne contient encore qu'un fichier vide ; l'implémentation (`UDPServer`, `UDPClient`, `ServerManager`, `ClientManager` et les messages du protocole) se trouve sur la branche `feature/network`.

---

## UDP ou TCP ?

Le choix du transport fait l'objet de l'[étude 10](10-protocole-reseau.md) : la partie se jouera en **UDP**, car avec TCP une seule perte de paquet bloque toutes les données suivantes jusqu'à son renvoi. TCP peut rester utile plus tard pour des échanges hors partie (salon, discussion). La bibliothèque doit donc offrir **les deux**.

---

## Critères

| Critère | Poids | Pourquoi il compte |
|---|---|---|
| **Windows et Linux** | 3 | [Étude 01](01-systemes-exploitation.md) : le même code réseau doit tourner sur les deux |
| **Modèle asynchrone** | 3 | Le serveur doit gérer plusieurs clients sans bloquer sa boucle de jeu |
| **Liberté du protocole** | 3 | Le protocole doit coller à l'état du jeu (composants de l'ECS) pour maîtriser la taille et la fréquence des paquets |
| **UDP et TCP** | 2 | UDP pour la partie, TCP éventuellement pour le salon |
| **Dépendances légères** | 2 | Récupérable par `FetchContent`, sans installation ([étude 03](03-systeme-de-build.md)) |
| **Maturité, documentation** | 2 | Le réseau est la partie la plus difficile à déboguer |

---

## Les options étudiées

### Asio autonome (retenu)

**Points forts**

- **Une API unique pour Windows et Linux.** Asio est testée sous Linux (GCC 4.8+, Clang 3.0+), macOS, Windows (Visual C++ 2013+) et FreeBSD ([Asio](https://think-async.com/Asio/)). Elle utilise en interne le meilleur mécanisme de chaque système (`epoll`, IOCP, `kqueue`).
- **Asynchrone :** `async_receive_from` et `async_send_to` reçoivent une fonction appelée à la fin de l'opération. Un seul thread, ou un petit groupe de threads, sert tous les clients.
- **UDP, TCP, minuteries et résolution de noms** : tout ce qu'il faut pour un serveur de jeu.
- **Sans Boost :** selon sa documentation, avec C++11 ou plus, « *la plupart d'Asio peut être utilisée sans aucune dépendance supplémentaire* » ([Asio](https://think-async.com/Asio/)). Elle est composée uniquement d'en-têtes : rien à compiler à part.
- **La plus éprouvée :** créée en 2003, intégrée à Boost en 2005, selon Wikipédia ([*Asio*](https://en.wikipedia.org/wiki/Asio_(C%2B%2B_library))). Le *Networking TS* (ISO/IEC TS 19216:2018), extension réseau du C++, a une conception « *basée sur la bibliothèque Asio* » ([cppreference, *Networking TS*](https://en.cppreference.com/w/cpp/experimental/networking)).
- **Bas niveau :** elle transporte des octets, sans imposer de format de paquet. Le protocole reste entièrement à concevoir, au plus près des besoins du jeu.

**Points faibles**

- **Courbe d'apprentissage :** le modèle asynchrone (callbacks, durée de vie des tampons, `io_context`) est exigeant.
- **Beaucoup d'en-têtes :** inclure `asio.hpp` ralentit la compilation des fichiers concernés.
- **Fiabilité à construire soi-même** sur UDP (numéros de séquence, accusés de réception) quand elle est nécessaire.

### Boost.Asio

**Points forts**

- Le même code (Boost.Asio 1.38.2 est livrée avec Boost 1.92.0), avec l'écosystème Boost (Beast pour HTTP, par exemple).

**Points faibles**

- **Dépend de Boost**, une bibliothèque énorme, longue à télécharger et à compiler, pour un gain nul ici.

### Les sockets natifs (BSD + Winsock), enveloppés à la main

**Points forts**

- **Aucune dépendance**, et un contrôle total.

**Points faibles**

- **Deux implémentations à écrire et à tester** (BSD et Winsock), plus un mécanisme d'attente par système (`epoll`, IOCP) pour ne pas bloquer. C'est réécrire une partie d'Asio, avec moins de tests.

### ENet

**Points forts**

- **Pensée pour les jeux :** une bibliothèque C qui ajoute sur UDP une livraison fiable et ordonnée optionnelle, des canaux et la fragmentation des gros paquets ([ENet, *Features and Architecture*](https://github.com/lsalzman/enet/blob/master/docs/design.dox)).

**Points faibles**

- **Impose son propre protocole** (en-têtes, accusés, canaux), qui s'ajoute à celui du jeu et en fixe une partie des choix.
- **UDP uniquement**, et un modèle de boucle de service à interroger, pas d'asynchrone.

### GameNetworkingSockets (Valve)

**Points forts**

- **Très complète :** messages fiables et non fiables sur UDP, chiffrement AES-GCM-256 par paquet, traversée de NAT pour le pair-à-pair, outils de simulation de latence et de perte ([GameNetworkingSockets](https://github.com/ValveSoftware/GameNetworkingSockets)).

**Points faibles**

- **Dépendances lourdes :** une bibliothèque cryptographique (OpenSSL 1.1.1+ ou libsodium), Google Protobuf dont le compilateur et la bibliothèque doivent être « *exactement de la même version* », et Abseil pour les versions récentes de Protobuf ([BUILDING.md](https://github.com/ValveSoftware/GameNetworkingSockets/blob/master/BUILDING.md)).
- **Impose son protocole**, comme ENet.

### SFML Network

**Points forts**

- Une API C++ simple pour UDP et TCP.

**Points faibles**

- **Il faudrait embarquer SFML** alors que l'affichage utilise raylib ([étude 05](05-bibliotheque-graphique.md)).
- Sockets bloquants ou non bloquants avec un sélecteur, sans vrai modèle asynchrone.

### Le réseau standard du C++ ?

**Il n'existe pas.** Le *Networking TS*, basé sur Asio, n'a pas été intégré au C++20, malgré une proposition en ce sens de l'auteur d'Asio ([P1446R0](https://wg21.link/p1446)). Le C++26 adopte `std::execution`, un cadre de programmation asynchrone ([Wikipédia, *Asio*](https://en.wikipedia.org/wiki/Asio_(C%2B%2B_library))), mais la liste des en-têtes de la bibliothèque standard ne contient toujours **aucun en-tête réseau** ([cppreference, *Standard library headers*](https://en.cppreference.com/w/cpp/header)). Aujourd'hui, une bibliothèque tierce reste indispensable.

---

## Les données

| Bibliothèque | Étoiles GitHub | Licence | Transport | Protocole imposé | Dépendances |
|---|---|---|---|---|---|
| **Asio** | 6 220 | Boost Software License | UDP, TCP | Non | Aucune (C++11) |
| GameNetworkingSockets | 9 989 | BSD-3-Clause | UDP (messages) | Oui | OpenSSL ou libsodium, Protobuf |
| ENet | 3 291 | MIT | UDP | Oui | Aucune |

*Source : API GitHub, consultée le 19 octobre 2026.*

---

## Grille de comparaison

| Critère (poids) | Asio autonome | Boost.Asio | Sockets natifs | ENet | GameNetworkingSockets | SFML Network |
|---|---|---|---|---|---|---|
| Windows et Linux (3) | 5 | 5 | 2 | 5 | 4 | 5 |
| Modèle asynchrone (3) | 5 | 5 | 2 | 2 | 3 | 2 |
| Liberté du protocole (3) | 5 | 5 | 5 | 3 | 2 | 5 |
| UDP et TCP (2) | 5 | 5 | 5 | 3 | 3 | 5 |
| Dépendances légères (2) | 4 | 2 | 5 | 5 | 1 | 2 |
| Maturité, documentation (2) | 5 | 5 | 3 | 4 | 5 | 4 |
| **Total pondéré /100** | **97** | **92** | **71** | **72** | **60** | **77** |

---

## Décision et conséquences

**`engine-network` reposera sur Asio autonome**, avec les définitions déjà en place (`ASIO_STANDALONE`, `ASIO_NO_DEPRECATED`, Windows 10 minimum, `ws2_32` et `mswsock` sous Windows).

**Asio est enveloppée dans `engine-network`.** Sur la branche `feature/network`, `UDPServer` et `UDPClient` utilisent Asio, et le jeu passe par `ServerManager` et `ClientManager`, deux composants dont les systèmes traitent le réseau à chaque frame. Cette séparation vise trois avantages :

1. **Le jeu ne dépend d'aucune API réseau :** il ne connaît ni Asio, ni Winsock, ni les sockets BSD.
2. **Asio ne croise pas raylib** dans un même fichier : sous Windows, Asio inclut les en-têtes Win32, dont certains noms entrent en conflit avec raylib ([étude 05](05-bibliotheque-graphique.md#décision-et-conséquences)).
3. **On peut changer de bibliothèque** en ne réécrivant que `engine-network`.

> [!WARNING]
> Ce n'est pas encore tout à fait le cas : `Network.hpp` inclut `<asio.hpp>`, et `KeyboardProtocol.hpp` inclut à la fois `Network.hpp` et `KeyboardManager.hpp` (donc raylib). Asio et raylib se retrouvent ainsi dans les mêmes fichiers, y compris dans le `Main.cpp` du client. Il faudra vérifier que cela compile sous Windows, ou sortir `<asio.hpp>` des en-têtes publics (en ne l'incluant que dans les `.cpp` du réseau).

**Le protocole** (UDP, en-tête de 9 octets, connexion par identifiant, contrôle de présence) est décrit dans l'[étude 10](10-protocole-reseau.md).

**Ce que l'on accepte :** l'apprentissage du modèle asynchrone, et la fiabilité à construire soi-même sur UDP là où elle est nécessaire.

---

## Quand revoir ce choix

- **Asio devient un frein** (temps de compilation, modèle asynchrone trop complexe pour le besoin) : écrire l'abstraction au-dessus des sockets BSD et Winsock (l'option « sockets natifs »), en gardant la même interface.
- **Le jeu a besoin de chiffrement, de traversée de NAT ou de pair-à-pair** : GameNetworkingSockets devient intéressante malgré ses dépendances.
- **Le C++ adopte un vrai réseau standard** (au-delà de `std::execution`) : migrer vers lui.

---

## Sources

- Asio, page officielle : <https://think-async.com/Asio/>
- Wikipédia, *Asio (C++ library)* : <https://en.wikipedia.org/wiki/Asio_(C%2B%2B_library)>
- cppreference, *Networking TS* : <https://en.cppreference.com/w/cpp/experimental/networking>
- cppreference, *Standard library headers* : <https://en.cppreference.com/w/cpp/header>
- ENet, *Features and Architecture* : <https://github.com/lsalzman/enet/blob/master/docs/design.dox>
- Christopher Kohlhoff, P1446R0, proposition d'intégrer le Networking TS au C++20 : <https://wg21.link/p1446>
- ENet : <https://github.com/lsalzman/enet>
- Valve, GameNetworkingSockets : <https://github.com/ValveSoftware/GameNetworkingSockets>
- Valve, GameNetworkingSockets, *BUILDING.md* : <https://github.com/ValveSoftware/GameNetworkingSockets/blob/master/BUILDING.md>
- API GitHub (étoiles et licences), consultée le 19 octobre 2026
