# Alpine : le terminal en mode graphique

Cette documentation a pour but d'apprendre à configurer Alpine Linux en mode graphique

----

### Prérequis

- ISO Alpine Linux : `alpine_virt_3.22.2`
- Oracle VirtualBox sous sa dernière version.

---

## 2. Alpine

Alpine est une distribution graphique très légère. Cette légèreté permet de faire fonctionner en parallèle plusieurs machines virtuelles Alpine sur un ordinateur portable.

On apprendra à installer Alpine Linux en graphique.

---

### 2.1 Création de la machine virtuelle

Créons une machine virtuelle avec Alpine Linux. 

Sur VirtualBox:

1. Nouvelle

2. Préciser un nom de machine virtuelle : `AlpineLinux-og`

3. Renseigner l'ISO d'Alpine Linux : 

4. Choisir les spécifications Hardware

   ![image-20251112212508550](/home/takagii/.config/Typora/typora-user-images/image-20251112212508550.png)

On laisse les spécifications par défaut pour notre première machine virtuelle.

5. Choisir les spécifications pour le disque virtuel puis confirmer les choix.

![image-20251112213036587](/home/takagii/.config/Typora/typora-user-images/image-20251112213036587.png)

Notre machine est prête pour son première démarrage.

![image-20251112213547509](/home/takagii/.config/Typora/typora-user-images/image-20251112213547509.png)

---

### 2.2 Installation et configuration de grub

Afin de pouvoir choisir si l'on souhaite démarrer en mode graphique `GUI` ou en mode console `CLI` il est nécessaire d'installer grub.

---

####  Installation

On démarre à la machine puis l'on se connecte : `localhost login: root`. On lance désormais le script d'installation avec la simple commande `setup-alpine`.

- On choisit une disposition de clavier : `fr`
- On choisit une variante : `fr-azerty`
- On choisit un nom d'hôte : `alpine`
- On choisit l'interface que nous souhaitons configurer : `eth0`
- On choisit la méthode de configuration : `dhcp`
- Configuration manuelle du réseau : `n`
- Changement du mot de passe root : `taka`
- Fuseau Horaire : `Europe/Paris`
- Proxy : `none`
- NTP : `busybox`
- APK Mirror : `(s)` puis `1` pour `dl-cdn.alpinelinux.org`
- User : `Adam` 
  - username : `ataha`
  - Password : `taki`

- SSH Server : `openssh`
- Disk & Install : `sda`
  - How would you like to use it : `sys`
- Erase the above disk(s) and continue (y/n) : `y`

Nous pouvons désormais éteindre notre machine virtuelle et éjecter le disque d'installation.

---

#### Mise  jour du système

```bash
apk update
apk upgrade
```

On active les dépôts communautaires en retirant le `#` dans le fichier `/etc/apk/repositories` : `vi /etc/apk/repositories`

```bash
https://dl-cdn.alpinelinux.org/alpine/v3.22/main
https://dl-cdn.alpinelinux.org/alpine/v3.22/community
```

---

#### Installation du bootloader GRUB

1. Installer GRUB :

- `apk add grub grub-bios`

2. Installer GRUB sur le disque

- `grub-install /dev/sda`

3. Récupération de l'UUID et PARTUUID

`awk '$2=="/"{print $1}' /proc/mounts`

`blkid -s UUID -o value /dev/sda1` : `1c71a0a4-069a-4d89-88a1-3874cf48240c` `6c0a2c39-3f59-4e03-86c0-01cb25951b11`

`blkid -s UUID -o value /dev/sda3` : `fb64b00f-e04c-4d56-9601-2cabdfd4e29d` `470b6fc3-8bb0-4766-96d5-e21396115f09`

`blkid -s PARTUUID -o value /dev/sda3` : `23b7e723-03` `2667a258-03`

---

#### Ajout des entrées dans GRUB

1. Modification du fichier : `vi /etc/grub.d/40_custom`

```bash
#!/bin/sh
exec tail -n +3 $0
# Custom GRUB entries

menuentry 'Alpine Linux (GUI)' {
    # Partition /boot = /dev/sda1
    search --no-floppy --fs-uuid --set=root 1c71a0a4-069a-4d89-88a1-3874cf48240c
    echo 'Mode Graphique...'
    linux /vmlinuz-virt root=UUID=fb64b00f-e04c-4d56-9601-2cabdfd4e29d ro modules=sd-mod,usb-storage,ext4 quiet rootfstype=ext4 softlevel=graphical
    echo 'Loading initial ramdisk...'
    initrd /initramfs-virt
}
```

2. Régénération de GRUB

- `grub-mkconfig -o /boot/grub/grub.cfg`

---

#### Installation de l'environnement graphique

1. Outil interactif : `setup-desktop`

On choisit `xfce`

---

#### Création du runlevel GUI

On ne souhaite pas que LightDM le mode graphique démarre automatiquement en mode "default". On crée un run level séparé nommé `gui`, qui sera invoqué seulement si l'on démarre en mode graphique par GRUB.

```bash
mkdir -p /etc/runlevels/graphical
rc-update add -s default graphical
rc-update add lightdm graphical
rc-update add dbus default      
rc-update del lightdm default
```

---

#### Remplacer Dropbear par OpenSSH (optionnel)

```bash
rc-update del dropbear
rc-service dropbear stop
apk del dropbear
apk add openssh
ssh-keygen -A
rc-update add sshd default
rc-service sshd start
```

---

### 2.3 Intégrer la suite logicielle

On souhaite intégrer des logiciels et outils nécessaires au bon fonctionnement et l'analyse du logiciel.

----

#### Installation des outils courants

```
apk add firefox-esr
apk add filezilla
apk add tcpdump
apk add wireshark
apk add putty
```

On ajoute l'utilisateur au groupe Wireshark : `adduser ataha wireshark`

---

#### Configuration sur la machine

1. Mettre le clavier en français : depuis l'interface graphique.

2. Installation de `zsh`

```bash
apk update
apk add zsh shadow
```

3. Mettre `zsh` en shell par défaut en root : `chsh` : `/bin/zsh`
4. Mettre `zsh` par défaut pour l'utilisateur : `chsh ataha` : `/bin/zsh`

5. Créer la config `.zshrc` : `vi /root/.zshrc`

```bash
# ----- Prompt -------------
PROMPT='%F{green}┌──(%B%F{blue}%n%F{white}@%F{blue}%m%b%F{green})-[%B%F{reset}%(6~.%-1~/…/%4~.%5~)%b%F{green}]
└─%B%(#.%F{red}#.%F{blue}$)%b%F{reset} '

# ----- Aliases utiles -----
alias ll="ls -alF"
alias la="ls -A"
alias l="ls -CF"
```

6. Copier la config pour l'utilisateur :

```bash
cp /root/.zshrc /home/ataha/.zshrc
chown ataha:ataha /home/ataha/.zshrc
```

---

### 2.4 Installer les services et modules VirtualBox

Installons les Guest Additions VirtualBox.

---

#### Additions VirtualBox

1. Installation des Guest Additions

```
apk add virtualbox-guest-additions virtualbox-guest-additions-x11
```

2. Ajout des Guest Additions au démarrage

```
rc-update add virtualbox-guest-additions boot
rc-update virtualbox add virtualbox-drm boot
```

3. Depuis VirtualBox : `General --> Advanced : Shared Clipboard & Drag'n'Drop : Bidirectionnal `

---

Voilà ! Il ne manque plus qu'à optimiser la taille mémoire et la taille du disque dur.
