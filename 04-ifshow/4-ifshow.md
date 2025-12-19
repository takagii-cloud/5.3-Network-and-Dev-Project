# ifshow : la commande pour lister les interfaces réseaux de la machine locale

Cette documentation a pour but de présenter le code `ifshow` développé en C.

### Prérequis

- Avoir un compilateur fonctionnel
- Machine de développement
- Machine de test
- Avoir fini les mini-projets 1,2 et 3
- Gestion de version par Git

## 4. ifshow : la commande pour lister les interfaces réseaux de la machine locale

ifshow est une commande en ligne à développer en langage C sous Linux. Elle affiche la liste des interfaces réseaux de la machine locale et leurs caractéristiques. Le comportement de la commande dépend des paramètres qui lui sont associées :

- `ifshow -i ifname` : affiche la liste des préfixes d'adresses IPv4 et IPv6 associées à l'interface nommée `ifname`. (un préfixe s'écrit sous la forme d.d.d.d/p pour l'IPv4 et d:d:d:d:d:d:d:d/p pour IPv6) ;

- `ifshow -a` : affiche la liste des noms des interfaces réseaux, et leurs préfixes d'adresses IPv4 et IPv6.

On apprendra à développer une petite application, sous la forme d'une commande en ligne, récupérant des informations systèmes à intégrer cette application dans divers environnements.

---

### 4.1 Développer la commande *ifshow* sur la machine hôte

```c

```



On utilise une Debian 13 pour développer, et une Debian 13 pour effectuer les tests.

### 4.2 Intégrer la commande *ifshow* à la machine virtuelle VyOS du mini-projet 1



### 4.3 Intégrer la commande *ifshow* à la machine virtuelle MicroCore du mini-projet 2



### 4.4 Intégrer la commande *ifshow* à la machine virtuelle MicroCore du mini-projet 3

