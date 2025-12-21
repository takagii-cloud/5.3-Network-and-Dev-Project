# ifnetshow : la commande pour lister les interfaces réseaux d'une machine distante

Cette documentation a pour but d'expliquer le script `ifnetshow`.

---

### Prérequis

- Compilateur fonctionnel
- Code fonctionnel `ifnetshow`
- Machine de développement
- Machine deux machines de test
- Avoir fini les mini-projets 1,2 et 3
- Gestion de versions par Git

---

## 6. *ifnetshow* : la commande pour lister les interfaces réseaux d'une machine distante

`ifnetshow` est une commande en ligne à développer en langage C sous Linux. Elle affiche la liste des interfaces réseaux d'une machine distante et leurs caractéristiques. Le comportement de la commande dépend des paramètres qui lui sont associées :

- `ifnetshow -n addr -i ifname` : affiche la liste des préfixes d'adresses IPv4 et IPv6 associées à l'interfaces nommée `ifname` de la machine ayant une interface réseau d’adresse IP `addr` ;
- `ifnetshow -n addr -a` : affiche la liste des noms des interfaces réseaux, et leurs préfixes d'adresses IPv4 et IPv6 de la machine ayant une interafce réseau d'adresse IP `addr`.

`ifnetshow` nécessite le développement d'un agent à installer sur machines distantes. L'agent devrait être persistant (il ne devrait pas être exécuté après chaque appel de `ifnetshow`).

On apprendra à développer une application communicante basée sur les sockets, en mode client/serveur (`ifnetshow` étant le client, et l'agent le serveur). On utilisera le réseau du mini-projet 5 pour valider le fonctionnement de `ifnetshow`. On réutilisera impérativement le code de `ifshow`. On apprendra à structurer votre programme pour le réutiliser.

---

### 6.1 Développer la commande *ifnetshow* et son agent sur le système hôte (persistant ou non). Utiliser le système de socket pour établir les communications entres les machines



---

### 6.2 Intégrer la commande *ifnetshow* et son agent (persistant ou non) au système VyOS du mini-projet 1



---

### 6.3 Intégrer la commande *ifnetshow* et son agent (persistant ou non) au système Alpine du mini-projet 2



---

### 6.4 Intégrer la commande *ifnetshow* et son agent (persistant ou non) au système MicroCore du mini-projet 3



