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



---

### 7.2 Intégrer la commande *neighborshow* aux systèmes VyOS, Alpine et MicroCore.



---

### 7.3 Développer une extension de *neighborshow* :

- `neighborshow -hop n` : affiche la liste des équipements en `n` sauts (si n=1, correspond à `neighborshow` sans paramètres)