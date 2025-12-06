# Microcore : le terminal en mode console

Cette documentation a pour but d'apprendre à configurer Microcore.

---

### Prérequis 

- ISO Tinycore 6.2 http://www.tinycorelinux.net/6.x/x86/archive/6.2/

- VirtualBox sous sa dernière version

---

## 3. Microcore

MicroCore est une distribution Linux en mode console très légère. On apprendra à l'installer, à configurer un système non persistent, à l'optimiser et à l'utiliser comme console virtuelle.

---

#### 3.1 Installer la machine Virtuelle

Créons une machine virtuelle intégrant le système Microcore.

Dans VirtualBox :

1. Nouvelle
2. Préciser un nom de machine virtuelle : `TinyCore-og`
3. Renseigner l'ISO de Microcore : ``TinyCore-6.2.iso``
4. Choisir les spécification Hardware 

Notre machine est prête pour son premier démarrage.

---

#### Premier démarrage

On choisit l'option `Boot TinyCore`.

---

On se connecte sans mot de passe lors de la première connexion. On télécharge ensuite les keymaps : `tce-load -wi kmaps`. Puis on change la disposition : `sudo loadkmap < /usr/share/kmap/azerty/fr.kmap`.

---

#### Installation persistante

On installe l'installateur TinyCore : `tce-load -wi tc-install`. Puis on lance ce dernier : `sudo tc-install.sh`

- Install from : `r`
- Enter boot directory : `/mnt/sr0/boot`
- Select Install type : `f`

- Select target for installation of core : `1. Whole Disk`
- Select disk for core : `1`
- Would you like to install a bootloader : `y`
- Select formatting Option : `laissons la ligne vide`

---

### 3.2 Intégration automatique du clavier azerty

1. Installation du paquet `kmaps` :

```bash
tce-load -wi kmaps
```

2. Charger la keymap française

```bash
sudo loadkmap < /usr/share/kmap/azerty/fr-latin1.kmap
```

---

### 3.3 Rendre `/home` et `/opt` persistants

1. Ajout de `/home` à la persistance

```bash
echo "/home" | sudo tee -a /opt/.filetool.lst
```

2. Ajout de `/opt` à la persistance

```bash
echo "/opt" | sudo tee -a /opt/.filetool.lst
```

3. Sauvegarde des données

```  
filetool.sh -b
```

----

### 3.4  Installer `IPv6`, `iproute2` et `tcpdump`

1. Installer `iproute2`

```bash
tce-load -wi iproute2
```

2. Installation de `tcpdump`

```bash
tce-load -wi tcpdump
```

3. Installation d'IPv6

```bash
tce-load -wi ipv6-3.16.6-tinycore
sudo modprobe ipv6
```

4. Vérification de l'IPv6

On finit par tout sauvegarder avec `filetool.sh -b`.