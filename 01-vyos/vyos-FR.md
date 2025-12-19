 # VyOS : le routeur virtuel

Cette documentation a pour but d'apprendre à configurer le routeur Virtuel VyOS 1.4.3 [sagitta]. 

----

### Prérequis 

- ISO VyOS fourni par l'établissement [vyos-1.4.3-generic-amd64.iso].
- Oracle VirtualBox sous sa dernière version.

----

## 1. VyOS 

Vyos est un système d'exploitation Linux basé sur Debian. Il dispose des mêmes services qu'un routeur matériel. S'il est installé sur une machine physique alors il opère en tant que routeur physique, et en tant que routeur virtuel dans le cas contraire.

On apprendra à installer VyOS et à configurer ce routeur virtuel.

----

### 1.1 Création de la machine virtuelle

Créons une machine virtuelle intégrant le système VyOS. Ce système sera installé sur le disque virtuel de la machine virtuelle. 

Sur VirtualBox :

1. Nouvelle.
2. Préciser un nom de machine virtuelle : `VyOS-og`
3. Renseigner l'ISO de VyOS.
4. Choisir les spécifications Hardware

 On laissera les spécifications par défaut pour notre première machine.

5. Choisir les spécifications pour le disque virtuel puis confirmer les choix.

6. Configuration : Réseau : On ajoute trois interfaces réseaux



Notre machine est prête pour son premier démarrage.

---

### 1.2 Intégrer automatiquement le clavier azerty

Avant de démarrer la machine pour éviter tout conflit on désactive temporairement les modules kvm.

```bash
sudo modprobe -r kvm_amd kvm
```

----

#### Première connexion

On commence par rentrer les identifiants `vyos login: vyos` et `Password: vyos`

----

#### Configuration du clavier temporaire

Par défaut le clavier est en qwerty, on change la disposition de la manière suivante :

```bash
sudo loadkeys fr
```

---

#### Début de l'installation

On lance l'installation de VyOS.

```bash
install image
```

On choisit l'option 1 de boot `/opt/vyatta/etc/config/config.boot`. Puis l'on éteint la machine virtuelle avec `poweroff`. Une fois éteinte il faut éjecter l'ISO qui nous servi à faire l'installation.

---

#### Configuration du clavier

Pour passer définitivement le clavier en Azerty/Français il suffit de lancer la commande suivante :

```bash
configure
set system option keyboard-layout fr
commit
save
```

On redémarre pour confirmer que la modification a bien été effectuée. Si c'est le cas nous pouvons passer à la suite.

Le clavier est bien configuré en français !

---

#### 1.3 Homogénéisation automatique des interfaces réseaux

Le but de cette partie est de configurer automatiquement l'homogénéisation des interfaces réseaux. Par exemple, la première interface réseau de la machine virtuelle sera `eth0`, la seconde `eth1` et ainsi de suite, soit au format `eth{0}` avec une incrémentation logique.

Si l'on clone notre premier routeur virtuel, le clone effectué devra prendre de nouvelles adresses MAC et commencer avec une première interface réseau `eth0` et ainsi de suite.

---

##### Tentative de clone

On essaie de faire un premier clone en générant de nouvelles adresses MAC pour toutes les interfaces réseau.

En démarrant notre clone on remarque le soucis suivant.

Les interfaces réseaux commencent par `eth4`. Du fait que notre routeur originel s'arrêtait à `eth3` car il avait 4 quatre interfaces. On peut désormais supprimer ce clone de test et tout les fichiers associés.

---

#### Suppression des adresses MAC des interfaces réseaux

Les adresses MAC d'une machine virtuelle clonée sont différentes des adresses MAC de l'interface de la machine virtuelle originelle. Les commandes suivantes permettent de s'assurer qu'aucune interface réseau n'ait des adresses MAC identiques.

```bash
vyos@vyos# del interfaces ethernet eth0 hw-id
[edit]
vyos@vyos# del interfaces ethernet eth1 hw-id
[edit]
vyos@vyos# del interfaces ethernet eth2 hw-id
[edit]
vyos@vyos# del interfaces ethernet eth3 hw-id
[edit]
vyos@vyos#
```

Or le système VyOS a une mauvaise interprétation de la modification d'une adresse MAC. S'il elle est modifiée il considère qu'un changement **physique** a été effectué. Et dans ce cas il continue à compter à partir de la fin de la machine virtuelle originelle comme dans le cas précédent.

Ainsi on démarre notre machine originelle `VyOS-og`. On passe en mode configuration avec `configure`. Puis l'on supprime les adresses MAC des 4 interfaces réseaux.

----

#### Nouvelle tentative de clone

On peut désormais faire notre nouveau clone avec pour `OS Installation Options : Générer de nouvelles adresses MAC pour toutes les interfaces réseau`.

On démarre notre nouveau clone afin de vérifier l'homogénéité des adresses. Les interfaces de la machine virtuelle commencent bien à partir de `eth0` et sont incrémentées de 1.

---

Nous avons bien fini le premier projet : VyOS le routeur virtuel.

 
