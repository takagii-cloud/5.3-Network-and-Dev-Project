# Projets Intégrateurs – Semestre 5

## 1. Objectifs Pédagogiques

Les projets intégrateurs ont pour objectifs de :

- Familiariser les étudiants avec les principaux outils utilisés dans le programme, notamment VMware, VirtualBox, Git, les outils de développement et les outils d’administration système et réseau.
- Mettre en pratique les notions de développement logiciel, systèmes d’exploitation et réseaux acquises au cours du semestre 5 ainsi que lors des semestres précédents.
- Développer l’autonomie, les compétences d’organisation et de gestion du temps.

---

## 2. Contraintes Générales

- Les machines virtuelles doivent être construites avec **VMware** ou **VirtualBox**.
- Tout développement logiciel doit être réalisé en **langage C**, avec **programmation socket** pour la partie réseau.
- Un gestionnaire de version **Git** doit être utilisé.
- Les projets sont **individuels** (pas de travail en groupe).

---

## 3. Mini-Projets

### 3.1 VyOS : Routeur Virtuel

- Créer une machine virtuelle basée sur **VyOS 1.4.3**, entièrement installée sur un disque virtuel.
- Configurer automatiquement le clavier en **AZERTY**.
- Renommer automatiquement les interfaces réseau lors du clonage des machines :
  - 1 interface : `eth0`
  - 2 interfaces : `eth0`, `eth1`
  - etc.

---

### 3.2 Alpine : Terminal Graphique Minimaliste

- Créer une machine virtuelle sous **Alpine Linux 3.22.2**.
- Installer et configurer **GRUB**, permettant un démarrage en mode console ou en mode graphique.
- Intégrer au minimum :
  - OpenSSH
  - Filezilla
  - Navigateur web
  - tcpdump
  - wireshark
  - putty
- Installer les **VMware Tools** afin de supporter l’intégration de la souris et l’ajustement automatique de la résolution d’écran.
- Optimiser le système afin de réduire :
  - l’utilisation de disque (`df`)
  - l’utilisation mémoire (`free`)

---

### 3.3 MicroCore : Système Console Minimaliste

- Créer une machine virtuelle MicroCore (**TinyCore Linux 6.2**).
- Configurer automatiquement le clavier en **AZERTY**.
- Rendre les répertoires `/home` et `/opt` **persistants**.
- Intégrer au minimum :
  - IPv6
  - La commande Linux `ip`
  - tcpdump

---

### 3.4 ifshow : Commande Locale d’Interfaces Réseau

- Développer la commande **ifshow** en C sous Linux.

**Fonctionnalités :**

- `ifshow -i ifname` : afficher les préfixes IPv4/IPv6 d’une interface donnée.
- `ifshow -a` : lister toutes les interfaces réseau locales avec leurs préfixes IPv4/IPv6 associés.

**Intégration sur :**

- Machine VyOS  
- Machine Alpine  
- Machine MicroCore

---

### 3.5 NetLab : Réseau Virtuel Intégré

- Construire un réseau virtuel incluant l’ensemble des mini-projets (routeurs VyOS et terminaux Alpine/MicroCore).
- Configurer le routage IPv4 permettant la communication complète entre les machines.
- Configurer le routage IPv6 avec **attribution automatique d’adresses sur les terminaux**.
- Les adresses réseau sont définies à partir d’un identifiant personnel **x** (fourni en cours).

---

### 3.6 ifnetshow : Inspection d’Interfaces Distantes

- Développer la commande **ifnetshow** ainsi que son agent serveur en C, utilisant la programmation socket.

**Fonctionnalités :**

- `ifnetshow -n addr -i ifname` : afficher les préfixes d’une interface distante.
- `ifnetshow -n addr -a` : afficher toutes les interfaces et leurs préfixes sur une machine distante.

**Contraintes supplémentaires :**

- L’agent doit être **persistant** si possible.
- Réutilisation du code de `ifshow`.
- Intégration sur :
  - VyOS
  - Alpine
  - MicroCore

---

### 3.7 neighborshow : Découverte des Voisins Réseau

- Développer la commande **neighborshow** en C.
- Afficher toutes les machines voisines présentes sur le même réseau physique, **sans connaissance préalable de leurs adresses**.
- Nécessite un agent fonctionnant sur les machines distantes.
- Les noms des systèmes doivent être récupérés dynamiquement via la commande Linux `hostname`.
- Intégration sur :
  - VyOS
  - Alpine
  - MicroCore

**Extension :**

- `neighborshow -hop n` : afficher les systèmes accessibles à *n* sauts.
