# 10 · UDP ou TCP, avec Asio ?

> 🌐 **Langue :** Français · [English](../en/10-network-protocol.md)

> **Décision :** la partie se joue en **UDP**, avec une connexion et un suivi des clients écrits au-dessus. Côté serveur, un seul `asio::ip::udp::socket` sert tous les clients.
> **Statut :** adopté, implémenté sur la branche `feature/network` · **Date :** octobre 2026 · [Retour au sommaire](README.md)

## En bref

**TCP** livre tout, dans l'ordre. Mais quand un paquet se perd, toutes les données suivantes attendent qu'il soit renvoyé : dans un jeu d'action, l'écran se fige puis rattrape son retard. **UDP** livre chaque datagramme dès qu'il arrive, ou jamais, et un datagramme est toujours un message complet. Le projet a donc choisi **UDP**, avec Asio. L'implémentation (branche `feature/network`) tient en un seul socket pour tous les joueurs, un en-tête de 9 octets, une connexion par identifiant et un contrôle de présence régulier. Les messages importants ne sont pas encore renvoyés en cas de perte : c'est la prochaine étape (voir [Décision](#décision-et-conséquences)).

---

## Contexte

`engine-network` repose sur Asio ([étude 06](06-bibliotheque-reseau.md)), qui propose les deux protocoles : « *Asio fournit un support prêt à l'emploi des protocoles Internet TCP, UDP et ICMP* » ([Asio, *TCP, UDP and ICMP*](https://think-async.com/Asio/asio-1.38.2/doc/asio/overview/networking/protocols.html)). Il faut choisir lequel transporte la partie.

Ce qui circule pendant une partie :

| Message | Sens | Fréquence | Peut-il se perdre ? |
|---|---|---|---|
| Entrées du joueur (touches appuyées) | client → serveur | Plusieurs dizaines de fois par seconde | Oui : les suivantes arrivent juste après |
| État du jeu (positions, vies…) | serveur → clients | Plusieurs dizaines de fois par seconde | Oui : le suivant le remplace |
| Connexion, début et fin de partie, mort d'un joueur | les deux | Rare | **Non** |

L'essentiel du trafic est donc fait de données qui **périment très vite**. Seule une petite partie doit arriver à coup sûr.

---

## Ce qui distingue TCP et UDP

| | **TCP** ([RFC 9293](https://www.rfc-editor.org/rfc/rfc9293.html)) | **UDP** ([RFC 768](https://www.rfc-editor.org/rfc/rfc768.html)) |
|---|---|---|
| **Livraison** | Garantie, dans l'ordre | Ni garantie, ni ordre |
| **Unité de données** | Un flux d'octets : « *il n'y a pas de frontières entre les messages* » ([Asio, *Streams*](https://think-async.com/Asio/asio-1.38.2/doc/asio/overview/core/streams.html)) | Un datagramme, toujours reçu en entier ou pas du tout |
| **En cas de perte** | Les données suivantes attendent le renvoi de la donnée perdue | Les datagrammes suivants arrivent normalement |
| **Connexion** | Établie explicitement, déconnexion détectée | Aucune : à gérer soi-même |
| **Taille de l'en-tête** | 20 octets au minimum | 8 octets |
| **Contrôle du débit** | Intégré (contrôle de congestion) | À faire soi-même : une application « *devrait contrôler le rythme* » de ses envois ([RFC 8085, section 3.1](https://www.rfc-editor.org/rfc/rfc8085.html#section-3.1)) |
| **Petits envois** | Peuvent être retardés pour être regroupés (algorithme de Nagle, [RFC 896](https://www.rfc-editor.org/rfc/rfc896.html)) | Partent immédiatement |

Le point décisif est le comportement en cas de perte. Glenn Fiedler, référence du réseau dans le jeu vidéo, le décrit ainsi pour TCP : les nouvelles données sont « *mises dans une file, et on ne peut pas y accéder tant que le paquet perdu n'a pas été retransmis* ». Il conclut : « *Utiliser TCP est la pire erreur possible quand on développe un jeu multijoueur* » ([Gaffer On Games, *UDP vs. TCP*](https://gafferongames.com/post/udp_vs_tcp/)).

---

## Ce que cela change dans le code Asio

### Avec TCP

- **Un `asio::ip::tcp::acceptor`** attend les connexions, et **chaque client a son propre `asio::ip::tcp::socket`**.
- **Il faut découper le flux soi-même**, puisqu'il n'a pas de frontières : en général, chaque message est précédé de sa taille, et on lit exactement ce nombre d'octets avec `asio::async_read`. La documentation d'Asio prévient que les lectures et écritures peuvent être partielles (« *short read or short write* ») ([Asio, *Streams*](https://think-async.com/Asio/asio-1.38.2/doc/asio/overview/core/streams.html)).
- **Il faut désactiver l'algorithme de Nagle** sur chaque socket avec `asio::ip::tcp::no_delay`, pour que les petits messages partent tout de suite ([Asio, *ip::tcp::no_delay*](https://think-async.com/Asio/asio-1.38.2/doc/asio/reference/ip__tcp/no_delay.html)).
- **En contrepartie**, la connexion, la déconnexion, l'ordre et la fiabilité sont fournis par le système.

### Avec UDP

- **Un seul `asio::ip::udp::socket`**, lié au port du serveur, reçoit les datagrammes de tous les clients.
- **`async_receive_from`** donne, pour chaque datagramme reçu, l'adresse de son expéditeur (son *endpoint*). Le serveur garde une table « endpoint → joueur » ([Asio, *TCP, UDP and ICMP*](https://think-async.com/Asio/asio-1.38.2/doc/asio/overview/networking/protocols.html)).
- **Un datagramme est un message complet** : pas de découpage à faire.
- **En contrepartie**, il n'y a ni connexion, ni ordre, ni fiabilité : c'est au jeu de les construire, là où il en a besoin.

Le cœur d'un serveur UDP avec Asio, sur le modèle de `UDPServer::startReceive` dans le projet :

```cpp
class UdpServer {
public:
    UdpServer(asio::io_context &io, std::uint16_t port) :
        _socket(io, asio::ip::udp::endpoint(asio::ip::udp::v4(), port))
    {
        receive();
    }

private:
    asio::ip::udp::socket _socket;
    asio::ip::udp::endpoint _sender;
    std::array<std::uint8_t, 1200> _buffer {};

    void receive()
    {
        _socket.async_receive_from(asio::buffer(_buffer), _sender,
            [this](std::error_code ec, std::size_t size) {
                if (!ec)
                    handlePacket(_sender, size);   // un datagramme = un message complet
                receive();
            });
    }

    void handlePacket(const asio::ip::udp::endpoint &from, std::size_t size);
};
```

---

## Critères

| Critère | Poids | Pourquoi il compte |
|---|---|---|
| **Latence en cas de perte** | 3 | Une perte ne doit pas figer la partie |
| **Fiabilité et ordre fournis** | 2 | Les messages importants doivent arriver |
| **Gestion des connexions** | 2 | Savoir qui est connecté, et détecter les départs |
| **Simplicité du code Asio** | 2 | Moins de code réseau, c'est moins de bugs difficiles à reproduire |
| **Surcoût par message** | 1 | En-têtes, regroupement des petits envois |
| **Charge du serveur** | 1 | Nombre de sockets et de lectures en cours |

---

## Les options étudiées

### TCP seul

**Points forts**

- Fiabilité, ordre, connexion et contrôle du débit fournis par le système.
- Pas de couche maison à écrire.

**Points faibles**

- **Une perte bloque tout le flux** jusqu'au renvoi : c'est exactement ce qu'un jeu d'action doit éviter.
- Découpage des messages à écrire (préfixe de taille), Nagle à désactiver, un socket par client.

### UDP seul, avec une couche maison (retenu)

**Points forts**

- **Une perte ne retarde rien :** le datagramme suivant arrive normalement.
- **Un datagramme est un message**, et un seul socket sert tous les clients.
- **On ne rend fiable que ce qui doit l'être** (connexion, événements), le reste part sans attente.

**Points faibles**

- **Connexion, déconnexion, accusés de réception et contrôle du débit à écrire.**
- Taille des datagrammes à limiter soi-même (voir [Décision](#décision-et-conséquences)).

### TCP et UDP ensemble (TCP pour les messages importants, UDP pour la partie)

**Points forts**

- Fiabilité gratuite pour la connexion et les événements, réactivité d'UDP pour la partie.

**Points faibles**

- **Deux protocoles, deux codes, deux sockets par client**, à synchroniser entre eux.
- **Les deux se gênent :** selon Glenn Fiedler, « *TCP a tendance à provoquer des pertes de paquets UDP* » ([Gaffer On Games](https://gafferongames.com/post/udp_vs_tcp/)). Les auteurs d'ENet, qui ont étudié cette combinaison pour leur jeu de tir *Cube*, concluent que le résultat est « *moins bon en fonctionnalités et en performance* » qu'un protocole unique au-dessus d'UDP ([ENet, *Features and Architecture*](https://github.com/lsalzman/enet/blob/master/docs/design.dox)).

---

## Les données

- **ENet** est né d'un jeu de tir multijoueur, *Cube*, qui envoyait des données très fréquemment avec une faible latence : « *TCP n'était pas un choix adapté, à cause de sa latence élevée et de son orientation flux* » ([ENet, *Features and Architecture*](https://github.com/lsalzman/enet/blob/master/docs/design.dox)).
- **Quake 3** repose essentiellement sur UDP : le serveur renvoie à chaque client la différence entre l'état actuel et le dernier état acquitté. Une information perdue est ainsi renvoyée naturellement dans le message suivant, sans canal fiable pour l'état du jeu ([Fabien Sanglard, *Quake 3 Source Code Review: Network Model*](https://fabiensanglard.net/quake3/network.php)).
- **Les recommandations de l'IETF pour UDP** demandent à une application de contrôler le rythme de ses envois, et de ne pas envoyer de datagrammes plus grands que ce que le chemin réseau accepte ([RFC 8085](https://www.rfc-editor.org/rfc/rfc8085.html)).

---

## Grille de comparaison

| Critère (poids) | TCP seul | UDP + couche maison | TCP + UDP |
|---|---|---|---|
| Latence en cas de perte (3) | 1 | 5 | 4 |
| Fiabilité et ordre fournis (2) | 5 | 2 | 4 |
| Gestion des connexions (2) | 5 | 2 | 4 |
| Simplicité du code Asio (2) | 3 | 4 | 2 |
| Surcoût par message (1) | 3 | 5 | 4 |
| Charge du serveur (1) | 3 | 5 | 3 |
| **Total pondéré /100** | **64** | **75** | **71** |

---

## Décision et conséquences

**La partie se joue en UDP.** C'est ce qu'implémentent `UDPServer` et `UDPClient` dans `Engine/Network`, sur la branche `feature/network` :

| Élément | Implémentation actuelle |
|---|---|
| **Sockets** | Un seul `asio::ip::udp::socket` côté serveur pour tous les clients, un par client |
| **En-tête (9 octets)** | `type` (`uint8`, le code du message), `payloadSize` (`uint16`), `sequence` (`uint32`), `senderId` (`uint16`), écrits octet par octet en gros-boutiste (`put16`, `put32`) |
| **Vérification** | Un datagramme plus court que l'en-tête, ou dont la taille ne correspond pas à `payloadSize`, est ignoré. Un message d'un identifiant inconnu, ou venant d'une autre adresse que celle enregistrée pour cet identifiant, est ignoré |
| **Connexion** | Le client envoie `LOGIN` toutes les 3 secondes jusqu'à recevoir son identifiant, attribué par le serveur. `LOGOUT` permet un départ explicite |
| **Présence** | Le serveur envoie `HEALTH` toutes les 2 secondes, le client répond. Un client silencieux depuis 6 secondes est retiré |
| **Messages du client** | Les entrées : `KEY_PRESSED`, `KEY_RELEASED`, `MOUSE_BUTTON_PRESSED`, `MOUSE_BUTTON_RELEASED` |
| **Messages du serveur** | Les changements d'entités : `CREATE`, `MOVE`, `SET`, `TEXT`, `DESTROY`, et les sons : `PLAY`, `PAUSE`, `STOP` |
| **Taille** | Tampon de réception de 1 024 octets : chaque datagramme reste sous les 1 200 octets recommandés |
| **Boucle de jeu** | Pas de thread réseau : un système de l'ECS (`ServerManager::systemServerReceived`, `ClientManager::systemClientReceived`) appelle `poll()` sur le contexte Asio à chaque frame |

Ces choix suivent les recommandations des sources de cette étude : le serveur reçoit des entrées, jamais des positions, et il ne croit pas le client sur parole ([Gambetta, *Client-Server Game Architecture*](https://www.gabrielgambetta.com/client-server-game-architecture.html)) ; les datagrammes restent sous 1 200 octets, la limite prudente retenue par QUIC ([RFC 9000, section 14](https://www.rfc-editor.org/rfc/rfc9000.html#section-14)) et par Glenn Fiedler ([Gaffer On Games, *Packet Fragmentation and Reassembly*](https://gafferongames.com/post/packet_fragmentation_and_reassembly/)).

**Ce que l'on accepte :** une couche de connexion à écrire et à tester, alors que TCP l'aurait fournie.

### Les prochaines étapes

L'implémentation actuelle n'a pas encore de fiabilité : un message perdu n'est jamais renvoyé. C'est sans conséquence pour `MOVE` (la position suivante le remplace) ou pour les entrées, mais pas pour tout :

1. **Rendre fiables `CREATE` et `DESTROY`.** Un `CREATE` perdu laisse le client sans l'entité, et les `MOVE` suivants la visent pour rien. Un `DESTROY` perdu laisse une entité fantôme à l'écran. Ces messages doivent être renvoyés tant qu'ils ne sont pas acquittés.
2. **Ajouter des accusés de réception.** Le numéro `sequence` existe déjà. Il reste à renvoyer, dans chaque paquet, le dernier numéro reçu et un champ de 32 bits qui indique lesquels des 32 paquets précédents sont arrivés ([Gaffer On Games, *Reliability and Congestion Avoidance over UDP*](https://gafferongames.com/post/reliability_ordering_and_congestion_avoidance_over_udp/)).
3. **Identifier le protocole.** Un identifiant de jeu et de version en tête de paquet permettrait d'ignorer d'emblée les datagrammes étrangers ou d'une ancienne version.
4. **Limiter le débit.** Le serveur doit plafonner le nombre de datagrammes acceptés par client et par seconde, et maîtriser son propre rythme d'envoi ([RFC 8085, section 3.1](https://www.rfc-editor.org/rfc/rfc8085.html#section-3.1)).

---

## Quand revoir ce choix

- **Le jeu ajoute des échanges hors partie volumineux et sans urgence** (salon de discussion, téléchargement de niveaux) : TCP devient pertinent pour ces flux-là, en gardant en tête qu'il peut perturber le trafic UDP.
- **La couche de fiabilité devient trop lourde à maintenir** : passer à une bibliothèque qui la fournit au-dessus d'UDP, comme ENet ou GameNetworkingSockets ([étude 06](06-bibliotheque-reseau.md)).
- **Le jeu doit tourner dans un navigateur** : un navigateur n'ouvre pas de socket UDP brut, il faudrait WebSocket ou WebRTC.

---

## Sources

- IETF, RFC 768, *User Datagram Protocol* : <https://www.rfc-editor.org/rfc/rfc768.html>
- IETF, RFC 9293, *Transmission Control Protocol (TCP)* : <https://www.rfc-editor.org/rfc/rfc9293.html>
- IETF, RFC 896, *Congestion Control in IP/TCP Internetworks* (John Nagle) : <https://www.rfc-editor.org/rfc/rfc896.html>
- IETF, RFC 8085, *UDP Usage Guidelines* : <https://www.rfc-editor.org/rfc/rfc8085.html>
- IETF, RFC 8200 (IPv6), taille minimale des liens : <https://www.rfc-editor.org/rfc/rfc8200.html>
- IETF, RFC 9000 (QUIC), section 14 *Datagram Size* : <https://www.rfc-editor.org/rfc/rfc9000.html#section-14>
- Asio, *TCP, UDP and ICMP* : <https://think-async.com/Asio/asio-1.38.2/doc/asio/overview/networking/protocols.html>
- Asio, *Streams, Short Reads and Short Writes* : <https://think-async.com/Asio/asio-1.38.2/doc/asio/overview/core/streams.html>
- Asio, *ip::tcp::no_delay* : <https://think-async.com/Asio/asio-1.38.2/doc/asio/reference/ip__tcp/no_delay.html>
- Glenn Fiedler, *UDP vs. TCP* : <https://gafferongames.com/post/udp_vs_tcp/>
- Glenn Fiedler, *Reliability and Congestion Avoidance over UDP* : <https://gafferongames.com/post/reliability_ordering_and_congestion_avoidance_over_udp/>
- Glenn Fiedler, *Packet Fragmentation and Reassembly* : <https://gafferongames.com/post/packet_fragmentation_and_reassembly/>
- ENet, *Features and Architecture* : <https://github.com/lsalzman/enet/blob/master/docs/design.dox>
- Fabien Sanglard, *Quake 3 Source Code Review: Network Model* : <https://fabiensanglard.net/quake3/network.php>
- Gabriel Gambetta, *Client-Server Game Architecture* : <https://www.gabrielgambetta.com/client-server-game-architecture.html>
