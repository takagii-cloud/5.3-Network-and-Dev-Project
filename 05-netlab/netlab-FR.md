# NetLab : le réseau virtuel

Cette documentation a pour but de mettre en place le réseau virtuel NetLab.

---

### Prérequis

- Avoir fini les projets 1, 2 et 3

- VirtualBox sous sa dernière version

---

## 5. NetLab le réseau Virtuel

NetLab est un réseau virtuel (voir figure 1) constitué de machines virtuelles conçues lors des mini-projet 1, 2 et 3. Les adresses des réseaux, et des interfaces réseaux, sont définies dans les tableaux 1 et 2. Nous devrons respecter rigoureusement les informations contenues dans ces tableaux. Vous apprendrez un configurer les équipements d’un petit réseau virtuel.

---

### 5.1 Configuration des équipements IPv4

Configurons dans un premier temps les équipements afin que tous les terminaux puissent communiquer entre eux en IPv4 avec protocole de routage statique ou dynamique.

---

#### Routeurs VyOS : R0, R1 et R2

On commence par cloner trois fois la machine VyOS-og créée lors de la première étape.

---

##### Commutateurs Virtuels

###### R0

Sur ce premier routeur on se rend dans les paramètres réseaux : VirtualBox --> Paramètres --> Réseau.

---

**Interface 1**

- Activer l'interface réseau 

- Mode d'accès : `Réseau interne`
- Nom : `N1`
- Câble connecté

Ce lien correspond à :

> R0 ↔ N1 ↔ T1
>  Réseau `10.x.1.0/24`

---

**Interface 2**

- Activer l'interface réseau 

- Mode d'accès : `Réseau interne`
- Nom : `N2`
- Câble connecté

Ce lien correspond à :

> R0 ↔ N2 ↔ T2
>  Réseau `10.x.2.0/24`

---

**Interface 3**

- Activer l'interface réseau 

- Mode d'accès : `Réseau interne`
- Nom : `N5`
- Câble connecté

Ce lien correspond à :

> R0 ↔ N5 ↔ R1
>  Réseau `192.168.x.16/28`

---

###### R1

Sur ce second routeur on effectue la même manipulation.

---

**Interface 1**

- Activer l'interface réseau 

- Mode d'accès : `Réseau interne`
- Nom : `N0`
- Câble connecté

Ce lien correspond à :

> R1 ↔ N0
>  Réseau `172.16.0.0/24`

---

**Interface 2**

- Activer l'interface réseau 

- Mode d'accès : `Réseau interne`
- Nom : `N4`
- Câble connecté

Ce lien correspond à :

> R1 ↔ N4 ↔ R2
>  Réseau `192.168.x.4/30`

---

**Interface 3**

- Activer l'interface réseau 

- Mode d'accès : `Réseau interne`
- Nom : `N5`
- Câble connecté

Ce lien correspond à :

> R1 ↔ N5 ↔ R0
>  Réseau `192.168.x.16/28`

---

###### R2

Sur ce troisième et dernier routeur on effectue la même manipulation.

---

**Interface 1**

- Activer l'interface réseau 

- Mode d'accès : `Réseau interne`
- Nom : `N3`
- Câble connecté

Ce lien correspond à :

> R2 ↔ N3 ↔T3/T4
>  Réseau `10.x.3.0/24`

---

**Interface 2**

- Activer l'interface réseau 

- Mode d'accès : `Réseau interne`
- Nom : `N4`
- Câble connecté

Ce lien correspond à :

> R2 ↔ N4 ↔ R1
>  Réseau `192.168.x.4/30`

---

##### Configuration IPv4 VyOS

Passons désormais à la configuration IPv4 de nos routeurs.

---

###### R0

```shell
configure
set system host-name 'R0'
set interfaces ethernet eth0 address 10.38.1.1/24
set interfaces ethernet eth1 address 10.38.2.1/24
set interfaces ethernet eth2 address 192.168.38.18/28
commit
save
exit
```

---

###### R1

```shell
configure
set system host-name 'R1'
set interfaces ethernet eth0 address 172.16.0.38/24
set interfaces ethernet eth1 address 192.168.38.5/30
set interfaces ethernet eth2 address 192.168.38.17/28
commit
save
exit
```

---

###### R2

```shell
configure
set system host-name 'R2'
set interfaces ethernet eth0 address 10.38.3.1/24
set interfaces ethernet eth1 address 192.168.38.6/30
commit
save
exit
```

---

##### Test de communication

###### R0 -> R1

```shell
ping -c1 192.168.38.17
```

---

###### R1 -> R0 et R2

```shell
ping 192.168.38.18
ping 192.168.38.6
```

---

###### R2 -> R1

```shell
ping 192.168.38.5
```

Les tests de communication sont censés être fonctionnels à ce niveau.

---

##### Routage IPv4

###### Sur R0

**R0** doit atteindre :`10.38.3.0/24` et `172.16.0.0/24` via **R1 (192.168.38.17)**

```shell
configure
set protocols static route 10.38.3.0/24 next-hop 192.168.38.17
set protocols static route 172.16.0.0/24 next-hop 192.168.38.17
set protocols static route 192.168.38.4/30 next-hop 192.168.38.17
commit
save
exit
```

---

###### Sur R1

**R1** doit atteindre `10.38.1.0/24` et `10.38.2.0/24` via **R0 (192.168.38.18)**, et `10.38.3.0/24` via **R2 (192.168.38.6)**.

```shell
configure
set protocols static route 10.38.1.0/24 next-hop 192.168.38.18
set protocols static route 10.38.2.0/24 next-hop 192.168.38.18
set protocols static route 10.38.3.0/24 next-hop 192.168.38.6
commit
save
exit
```

---

###### Sur R2

**R2** doit atteindre `10.38.1.0/24`, `10.38.2.0/24` et `172.16.0.0/24` via **R1 (192.168.38.5)**.

```shell
configure
set protocols static route 10.38.1.0/24 next-hop 192.168.38.5
set protocols static route 10.38.2.0/24 next-hop 192.168.38.5
set protocols static route 172.16.0.0/24 next-hop 192.168.38.5
set protocols static route 192.168.38.16/28 next-hop 192.168.38.5
commit
save
exit
```

---

##### Test de communication Post Routage IPv4

###### R0

```shell
ping 10.38.3.1
ping 172.16.0.38
```

---

###### R1

```shell
ping 10.38.1.1
ping 10.38.2.1
ping 10.38.3.1
```

---

###### R2

```shell
ping 10.38.1.1
ping 10.38.2.1
```

Le routage IPv4 entre les routeurs fonctionnent correctement.

---

#### Terminaux virtuels MicroCore : T1, T2 et T3

On commence par cloner notre machine MicroCore-og créée lors de la deuxième étape.

---

 ##### Interfaces réseaux

###### T1

Sur cette première machine on se rend dans les paramètres réseaux : VirtualBox --> Paramètres --> Réseau.

---

**Interface 1**

- Activer l'interface réseau
- Mode d'accès : `Réseau interne`
- Nom : `N1`
- Câble connecté

Ce lien correspond à :

> T1 ↔ R0
>  Réseau `10.x.1.1/24`

---

###### T2

On répète la même manipulation.

---

**Interface 1**

- Activer l'interface réseau
- Mode d'accès : `Réseau interne`
- Nom : `N2`
- Câble connecté

Ce lien correspond à :

> T2 ↔ R0
>  Réseau `10.x.2.1/24`

---

###### T3

On répète une dernière fois cette manipulation

---

**Interface 1**

- Activer l'interface réseau
- Mode d'accès : `Réseau interne`
- Nom : `N3`
- Câble connecté

Ce lien correspond à :

> T3 ↔ R2
>  Réseau `10.x.3.1/24`

---

##### Configuration Microcore

Passons désormais à la configuration IPv4 de nos terminaux.

---

###### T1

On adresse la machine et l'on ajoute une route par défaut.

```shell
sudo ip addr add 10.38.1.101/24 dev eth0
sudo ip link set eth0 up
sudo ip route add default via 10.38.1.1
```

---

###### T2

On adresse de la même manière la machine T2 en ajoutant une route par défaut.

```shell
sudo ip addr add 10.38.2.102/24 dev eth0
sudo ip link set eth0 up
sudo ip route add default via 10.38.2.1
```

---

###### T3

On effectue la même procédure sur T3.

```shell
sudo ip addr add 10.38.3.103/24 dev eth0
sudo ip link set eth0 up
sudo ip route add default via 10.38.3.1
```

---

##### Test de communication

###### T1->T2 & T3

```shell
ping -c1 10.38.2.102
ping -c1 10.38.3.103
```

----

###### T2 -> T1 & T3

```shell
ping -c1 10.38.1.101
ping -c1 10.38.3.103
```

---

###### T3 -> T1 & T2

```shell
ping -c1 10.38.1.101
ping -c1 10.38.2.102
```

Les pings sont tous fonctionnels.

---

##### Persistance

Sur les trois machines on créé un fichier `/opt/bootsync.sh` avec les commandes précédentes.

---

#### Terminal virtuel T1

On commence par cloner notre machine virtuelle Alpine-og créé lors de la deuxième étape.

---

##### Interface réseau

On se rend dans les paramètres réseaux.

---

**Interface 1**

- Activer l'interface réseau
- Mode d'accès : `Réseau interne`
- Nom : `N1`
- Câble connecté

Ce lien correspond à :

> T4 ↔ R2
> Réseau `10.x.3.104/24`

---

##### Configuration IPv4

```shell
doas ip addr add 10.38.3.104/24 dev eth0
doas ip link set eth0 up
doas ip route add default via 10.38.3.1
```

---

##### Persistance

On utilise `doas setup-interfaces et l'on répond aux questions`. Puis l'on exécute `sudo rc-service networking restart`

##### Test de communication

###### T4 -> T1, T2 & T3 

```shell
ping 10.38.1.101
ping 10.38.2.102
ping 10.38.3.103
```

Les tests de communication sont bien fonctionnels.

----

### 5.2 Configuration des équipements IPv6

Configurons dans un second temps les équipements afin que tous les terminaux puissent communiquer entre eux en IPv6 avec protocole de routage statique ou dynamique.

----

#### Routeurs VyOS : R0, R1 et R2

On réalise la même procédure cette fois-ci en IPv6.

---

##### Configuration IPv6 VyOS

Premièrement adressons nos routeurs en IPv6.

---

###### R0

```shell
configure
set interfaces ethernet eth0 address 2001:0:38:1::1/64
set interfaces ethernet eth1 address 2001:0:38:2::1/64
set interfaces ethernet eth2 address 3ffe:0:38:16::2/64
commit
save
exit
```

---

###### R1

```shell
configure
set interfaces ethernet eth0 address 2002:16:0:0::38/64
set interfaces ethernet eth1 address 3ffe:0:38:4::1/64
set interfaces ethernet eth2 address 3ffe:0:38:16::1/64
commit
save
exit
```

---

###### R2

```shell
configure
set interfaces ethernet eth0 address 2001:0:38:3::1/64
set interfaces ethernet eth1 address 3ffe:0:38:4::2/64
commit
save
exit
```

---

##### Test de communication

###### R0 -> R1

```shell
ping6 3ffe:0:38:16::1
```

---

###### R1 -> R0 & R2

```shell
ping6 3ffe:0:38:16::2
ping6 3ffe:0:38:4::2
```

---

###### R2 -> R1

```shell
ping6 3ffe:0:38:4::1
```

Les tests sont fonctionnels.

---

##### Routage IPv6

###### Sur R0

```shell
configure
set protocols static route6 2001:0:38:3::/64 next-hop 3ffe:0:38:16::1
set protocols static route6 2002:16:0:0::/64 next-hop 3ffe:0:38:16::1
set protocols static route6 3ffe:0:38:4::/64 next-hop 3ffe:0:38:16::1
commit
save
exit
```

---

###### Sur R1

```shell
configure
set protocols static route6 2001:0:38:1::/64 next-hop 3ffe:0:38:16::2
set protocols static route6 2001:0:38:2::/64 next-hop 3ffe:0:38:16::2
set protocols static route6 2001:0:38:3::/64 next-hop 3ffe:0:38:4::2
commit
save
exit
```

---

###### Sur R2

```shell
configure
set protocols static route6 2001:0:38:1::/64 next-hop 3ffe:0:38:4::1
set protocols static route6 2001:0:38:2::/64 next-hop 3ffe:0:38:4::1
set protocols static route6 2002:16:0:0::/64 next-hop 3ffe:0:38:4::1
set protocols static route6 3ffe:0:38:16::/64 next-hop 3ffe:0:38:4::1
commit
save
exit
```

---

##### Test de communication IPv6

###### R0

```shell
ping6 2001:0:38:3::1
ping6 2002:16:0:0::38
```

---

###### R1

```shell
ping6 2001:0:38:1::1
ping6 2001:0:38:2::1
ping6 2001:0:38:3::1
```

---

###### R2

```shell
ping6 2001:0:38:1::1
ping6 2001:0:38:2::1
```

Les tests sont bien fonctionnels.

---

##### Activation des Router Avertissements (SLAAC)

###### R0 

```shell
configure
set service router-advert interface eth0 prefix 2001:0:38:1::/64
set service router-advert interface eth1 prefix 2001:0:38:2::/64
commit
save
exit
```

---

###### R2

```shell
configure
set service router-advert interface eth0 prefix 2001:0:38:3::/64
commit
save
exit
```

Sur **VyOS**, l’auto-configuration IPv6 des terminaux se fait via **Router Advertisements (RA)**, configurés **au niveau des interfaces**.

---

##### Terminaux 

Sur les 4 terminaux on fait `ip a` et l'on vérifie que les terminaux ont bien reçu des IP en IPv6 automatique. C'est bien le cas.

---

##### Test de connectivité 

###### T1

```shell
ping6 -c1 2001:0:38:2:a00:27ff:fe68:266c # T2
ping6 -c1 2001:0:38:3:a00:27ff:fe29:4eaf # T3
ping6 -c1 2001:0:38:3:a00:27ff:fe34:e5f0 # T4
```

---

###### T2

```shell
ping6 -c1 2001:0:38:1:a00:27ff:fe23:e052 # T1
ping6 -c1 2001:0:38:3:a00:27ff:fe29:4eaf # T3
ping6 -c1 2001:0:38:3:a00:27ff:fe34:e5f0 # T4
```

---

###### T3

```shell
ping6 -c1 2001:0:38:1:a00:27ff:fe23:e052 # T1
ping6 -c1 2001:0:38:2:a00:27ff:fe68:266c # T2
ping6 -c1 2001:0:38:3:a00:27ff:fe34:e5f0 # T4
```

---

###### T4

```shell
ping6 -c1 2001:0:38:1:a00:27ff:fe23:e052 # T1
ping6 -c1 2001:0:38:2:a00:27ff:fe68:266c # T2
ping6 -c1 2001:0:38:3:a00:27ff:fe29:4eaf # T3
```

Les tests sont fonctionnels.

---

### Annexe : Le réseau NetLab

### ![image-20251213172349254](/home/takagii/.config/Typora/typora-user-images/image-20251213172349254.png) 

Le réseau est constitué des équipements suivants :

- T1, T2 et T3 des terminaux virtuels Microcore
- T4 un terminal virtual Alpine
- R0, R1 et R2 des routeurs virtuels VyOS
- N0, N1, N2, N3, N4 et N5 des commutateurs virtuels (intégrés à l'hyperviseur)

Le tableau 1 définit les adresses des réseaux.

Le tableau 2 définit les adresses d'interfaces des équipements. Les adresses d'interface IPv6 des équipements terminaux ne sont pas spécifiées, car leurs configurations doivent être automatiques.

| Réseaux | Adresses réseaux                          |
| :------ | ----------------------------------------- |
| N0      | `172.16.0.0/24`<br />`2002:16:0:0::/64`   |
| N1      | `10.x.1.0/24`<br />`2001:0:x:1::/64`      |
| N2      | `10.x.2.0/24`<br />`2001:0:x:2::/64`      |
| N3      | `10.x.3.0/24`<br />`2001:0:x:3::/64`      |
| N4      | `192.168.x.4/30`<br />`3FFE:0:x:4::/64`   |
| N5      | `192.168.x.16/28`<br />`3FFE:0:x:16::/64` |

*Tableau 1 : Adresses des réseaux de NetLab*



|      | Interfaces | Adresses interfaces                        |
| ---- | ---------- | ------------------------------------------ |
| R0   | N1         | `10.x.1.1/24`<br />`2001:0:x:1::1/64`      |
| R0   | N2         | `10.x.2.1/24`<br />`2001:0:x:2::1/64`      |
| R0   | N5         | `192.168.x.18/28`<br />`3FFE:0:x:16::2/64` |

|      | Interfaces | Adresses interfaces                        |
| ---- | ---------- | ------------------------------------------ |
| R1   | N0         | `172.16.0.x/24`<br />`2002:16:0:0::x/64`   |
| R1   | N4         | `192.168.x.5/30`<br />`3FFE:0:x:4::1/64`   |
| R1   | N5         | `192.168.x.17/28`<br />`3FFE:0:x:16::1/64` |

|      | Interfaces | Adresses interfaces                      |
| ---- | ---------- | ---------------------------------------- |
| R2   | N3         | `10.x.3.1/24`<br />`2001:0:x:3::/64`     |
| R2   | N4         | `192.168.x.6/30`<br />`3FFE:0:x:4::2/64` |



|      | Interfaces | Adresses interfaces |
| ---- | ---------- | ------------------- |
| T1   | N1         | `10.x.1.101/24`     |
| T2   | N2         | `10.x.2.102/24`     |
| T3   | N3         | `10.x.3.103/24`     |
| T4   | N3         | `10.x.3.104/24`     |

*Tableau 2 : Adresses des interfaces réseaux des équipements de NetLab*