# neighborshow : la commande pour lister l'ensemble des machines voisines

Cette documentation a pour but d'expliquer le script `neighborshow`

---

### Prérequis

- Compilateur fonctionnel
- Machine de développement
- Machine de test
- Projets 1,2, 3 et 5 fonctionnels
- Gestion de Versions via Git

---

## 7. neighborshow : la commande pour lister l'ensemble des machines voisines.

`neighborshow` est une commande en ligne à développer en langage C sous Linux. Elle affiche la liste des noms (sans doublon) des machines voisines (machines d'un même réseau physique) sans connaissance préalable de leurs adresses respectives.

`neighborshow` nécessite le développement d'un agent à installer sur les machines distantes. L'agent devrait être persistant (il ne devrait pas être exécuté après chaque appel de `neighborshow`)

On apprendra à développer une petite application communicante complexe basée sur les sockets. On utilisera le réseau du mini-projet 5 pour valider le fonctionnement de `netshow`.

---

### 7.1 Développer la commande *neighborshow* sur le système hôte. Utiliser le système de socket pour établir les communications entre les machines. La liste des noms est dynamique. Les noms correspondent au nom système (voir commande linuxhostname)

### Agent Serveur `neighborshowd.c`

On crée un agent serveur **neighborshowd.c**.

- écoute en UDP sur **9091**
- écoute **en IPv4 ET IPv6** (2 sockets → robuste en VM)
- répond à toute requête `NSHOW? <nonce>` par `NSHOW! <nonce> <hostname>`

```c
#define _GNU_SOURCE
#define _DEFAULT_SOURCE

#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define UDP_PORT_STR "9091"
#define MAX_LINE 512
#define MAX_HOST 256

static void get_my_hostname(char *out, size_t cap) {
  if (gethostname(out, cap) != 0) strncpy(out, "unknown", cap);
  out[cap - 1] = '\0';
  for (char *p = out; *p; p++) {
    if (*p == '.') { *p = '\0'; break; }
  }
}

static int make_udp_server(int family) {
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = family;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;

  struct addrinfo *res = NULL;
  int rc = getaddrinfo(NULL, UDP_PORT_STR, &hints, &res);
  if (rc != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc));
    return -1;
  }

  int s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (s < 0) { freeaddrinfo(res); return -1; }

  int yes = 1;
  setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

  if (bind(s, res->ai_addr, res->ai_addrlen) < 0) {
    close(s);
    freeaddrinfo(res);
    return -1;
  }

  freeaddrinfo(res);
  return s;
}

static void handle_packet(int s, const char *myhost) {
  char buf[MAX_LINE];
  struct sockaddr_storage src;
  socklen_t slen = sizeof(src);

  ssize_t n = recvfrom(s, buf, sizeof(buf) - 1, 0, (struct sockaddr *)&src, &slen);
  if (n < 0) return;
  buf[n] = '\0';

  if (strncmp(buf, "NSHOW?", 6) != 0) return;

  // parse nonce after "NSHOW?"
  char nonce[128] = {0};
  const char *p = buf + 6;
  while (*p == ' ' || *p == '\t') p++;
  size_t i = 0;
  while (*p && *p != '\n' && *p != '\r' && i + 1 < sizeof(nonce)) {
    nonce[i++] = *p++;
  }
  nonce[i] = '\0';
  if (i == 0) return;

  char reply[MAX_LINE];
  snprintf(reply, sizeof(reply), "NSHOW! %s %s\n", nonce, myhost);
  (void)sendto(s, reply, strlen(reply), 0, (struct sockaddr *)&src, slen);
}

int main(void) {
  signal(SIGPIPE, SIG_IGN);

  char myhost[MAX_HOST];
  get_my_hostname(myhost, sizeof(myhost));

  int s4 = make_udp_server(AF_INET);
  int s6 = make_udp_server(AF_INET6);

  if (s4 < 0 && s6 < 0) {
    perror("neighborshowd: bind");
    return 1;
  }

  fprintf(stderr, "neighborshowd: listening UDP %s (hostname=%s)\n", UDP_PORT_STR, myhost);

  while (1) {
    fd_set rfds;
    FD_ZERO(&rfds);
    int maxfd = -1;

    if (s4 >= 0) { FD_SET(s4, &rfds); if (s4 > maxfd) maxfd = s4; }
    if (s6 >= 0) { FD_SET(s6, &rfds); if (s6 > maxfd) maxfd = s6; }

    int rc = select(maxfd + 1, &rfds, NULL, NULL, NULL);
    if (rc < 0) {
      if (errno == EINTR) continue;
      perror("select");
      break;
    }

    if (s4 >= 0 && FD_ISSET(s4, &rfds)) handle_packet(s4, myhost);
    if (s6 >= 0 && FD_ISSET(s6, &rfds)) handle_packet(s6, myhost);
  }

  if (s4 >= 0) close(s4);
  if (s6 >= 0) close(s6);
  return 0;
}
```

#### Explication

##### Récupération du hostname local

```c
static void get_my_hostname(char *out, size_t cap) {
  if (gethostname(out, cap) != 0) strncpy(out, "unknown", cap);
  out[cap - 1] = '\0';
  for (char *p = out; *p; p++) {
    if (*p == '.') { *p = '\0'; break; }
  }
}
```

- `gethostname()` lit le nom système
- on coupe au premier `.` pour éviter d’afficher un FQDN (ex: `vm1.lan.local` → `vm1`)
- ce `hostname` sera renvoyé au client

---

##### Création d'une socket UDP serveur (IPv4 ou IPv6)

```c
static int make_udp_server(int family) {
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = family;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;

  struct addrinfo *res = NULL;
  int rc = getaddrinfo(NULL, UDP_PORT_STR, &hints, &res);
  ...
  int s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  ...
  setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
  bind(s, res->ai_addr, res->ai_addrlen);
  ...
}
```

- `getaddrinfo(NULL, "9091", ...)` prépare une adresse “serveur” sur toutes les interfaces
- `SO_REUSEADDR` évite d’attendre si tu relances vite le daemon
- `bind()` attache le port UDP **9091**

On appelle cette fonction 2 fois :

- `s4 = make_udp_server(AF_INET)` → UDP IPv4
- `s6 = make_udp_server(AF_INET6)` → UDP IPv6

---

##### Traitement d'un paquet reçu `handle_packet`

```c
static void handle_packet(int s, const char *myhost) {
  recvfrom(...);      // lit le datagramme + récupère l’adresse source
  if (strncmp(buf, "NSHOW?", 6) != 0) return;

  // parse nonce
  ...
  snprintf(reply, sizeof(reply), "NSHOW! %s %s\n", nonce, myhost);
  sendto(s, reply, strlen(reply), 0, (struct sockaddr *)&src, slen);
}
```

- `recvfrom()` lit le message et récupère l’adresse IP/port du client
- on accepte uniquement les requêtes qui commencent par `NSHOW?`
- on extrait le `nonce` (identifiant unique envoyé par le client)
- on répond **en unicast** au client via `sendto()`

---

##### Boucle Principale `select`

```c
while (1) {
  FD_SET(s4) + FD_SET(s6);
  select(...);

  if (FD_ISSET(s4)) handle_packet(s4, myhost);
  if (FD_ISSET(s6)) handle_packet(s6, myhost);
}
```

- `select()` attend des paquets sur l’une ou l’autre socket
- dès qu’il y a un paquet, on le traite et on répond

---

### Résumé rapide (agent)

- `neighborshowd` écoute UDP/9091 en IPv4 + IPv6
- accepte `NSHOW? <nonce>`
- répond `NSHOW! <nonce> <hostname>`
- tourne en boucle : agent “persistant” possible via service au boot

---

### Client `neighborshow.c`

On crée un client **neighborshow.c**.

- génère un `nonce` unique
- diffuse une requête `NSHOW? <nonce>` sur le LAN :
  - broadcast IPv4 (global + calculé par interface)
  - multicast IPv6 link-local `ff02::1`
- écoute les réponses pendant ~1.5s
- affiche les hostnames reçus **sans doublons**

---

### Explication

##### Génération du hostname local

```c
char myhost[MAX_HOST];
get_my_hostname(myhost, sizeof(myhost));
```

Le client ignore les réponses qui contiennent son propre hostname.

---

##### Génération du `nonce` (anti-vieux paquets / anti-collisions)

```c
snprintf(nonce, sizeof(nonce), "%ld-%ld", (long)getpid(), (long)time(NULL));
snprintf(msg, sizeof(msg), "NSHOW? %s\n", nonce);
```

---

##### Création des sockets UDP client (IPv4 et IPv6)

```shell
int s4 = make_udp_v4();  // SO_BROADCAST activé
int s6 = make_udp_v6();  // IPv6
bind_ephemeral_v4(s4);
bind_ephemeral_v6(s6);
```

- IPv4 : `SO_BROADCAST` obligatoire pour envoyer en broadcast
- On fait un `bind` sur port éphémère pour recevoir les réponses

---

##### Envoi découverte IPv4 : broadcast global

```c
struct in_addr gb;
gb.s_addr = inet_addr("255.255.255.255");
send_ipv4_bcast(s4, gb, msg);
```

---

##### Envoie découverte IPv4 : broadcast calculé par interface

Dans la boucle `getiaddrs()` :

```c
broadcast = ip | (~netmask)
send_ipv4_bcast(s4, broadcast, msg);
```

Pourquoi pas `ifa_broadaddr` ?

- selon OS / driver / VM, `ifa_broadaddr` peut être vide ou non fiable
- calculer à partir de `ip` + `netmask` est beaucoup plus robuste

---

##### Envoi découverte IPv6 : multicast link-local

```shell
unsigned idx = if_nametoindex(it->ifa_name);
send_ipv6_allnodes(s6, idx, msg);
```

- `ff02::1` = multicast “tous les nœuds” scope **lien** (donc LAN)
- `scope_id` (index interface) est indispensable en link-local

---

##### Réception des réponses

```shell
select(...) sur s4 et s6
recvfrom(...)
```



Filtrage :

```c
if (strncmp(buf, "NSHOW!", 6) != 0) continue;
sscanf("NSHOW! %s %s", r_nonce, r_host);
if (strcmp(r_nonce, nonce) != 0) continue;
if (strcmp(r_host, myhost) == 0) continue;
```

- on accepte uniquement les réponses de ce lancement (`nonce`)
- on ignore sa propre machine

---

##### Dé-duplication

```c
add_unique(found, &fcount, MAX_NEIGH, r_host);
```

- on stocke les hostnames déjà vus
- on ne les ajoute pas deux fois
- affichage final de la liste

---

### 7.2 Intégrer la commande *neighborshow* aux systèmes VyOS, Alpine et MicroCore.



---

### 7.3 Développer une extension de *neighborshow* :

- `neighborshow -hop n` : affiche la liste des équipements en `n` sauts (si n=1, correspond à `neighborshow` sans paramètres)