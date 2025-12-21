# ifshow : la commande pour lister les interfaces réseaux de la machine locale

Cette documentation a pour but de présenter le code `ifshow` développé en C.

### Prérequis

- Avoir un compilateur fonctionnel
- Machine de développement
- Machine de test
- Avoir fini les mini-projets 1,2 et 3
- Gestion de version par Git

---

## 4. ifshow : la commande pour lister les interfaces réseaux de la machine locale

ifshow est une commande en ligne à développer en langage C sous Linux. Elle affiche la liste des interfaces réseaux de la machine locale et leurs caractéristiques. Le comportement de la commande dépend des paramètres qui lui sont associées :

- `ifshow -i ifname` : affiche la liste des préfixes d'adresses IPv4 et IPv6 associées à l'interface nommée `ifname`. (un préfixe s'écrit sous la forme d.d.d.d/p pour l'IPv4 et d:d:d:d:d:d:d:d/p pour IPv6) ;
- `ifshow -a` : affiche la liste des noms des interfaces réseaux, et leurs préfixes d'adresses IPv4 et IPv6.

On apprendra à développer une petite application, sous la forme d'une commande en ligne, récupérant des informations systèmes à intégrer cette application dans divers environnements.

### 4.1 Développer la commande *ifshow* sur la machine hôte

```c
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <ifaddrs.h>
 #include <arpa/inet.h>
 #include <netinet/in.h>
 #include <unistd.h>
 
 static int bitcount8(unsigned char v){ int n=0; while(v){ n += v&1u; v >>= 1u; } return n; }
 
 static int netmask_prefix(const struct sockaddr *mask){
   if(!mask) return 0;
 
   if(mask->sa_family == AF_INET){
     const unsigned char *m = (const unsigned char*)&((const struct sockaddr_in*)mask)->sin_addr;
     return bitcount8(m[0]) + bitcount8(m[1]) + bitcount8(m[2]) + bitcount8(m[3]);
   }
 
   if(mask->sa_family == AF_INET6){
     const unsigned char *m = (const unsigned char*)&((const struct sockaddr_in6*)mask)->sin6_addr;
     int p=0; for(int i=0;i<16;i++) p += bitcount8(m[i]); return p;
   }
 
   return 0;
 }
 
 static void print_ip_line(struct ifaddrs *e, int grouped){
   int af = e->ifa_addr->sa_family;
   char ip[INET6_ADDRSTRLEN];
 
   void *addr = (af==AF_INET)
     ? (void*)&((struct sockaddr_in*)e->ifa_addr)->sin_addr
     : (void*)&((struct sockaddr_in6*)e->ifa_addr)->sin6_addr;
 
   if(!inet_ntop(af, addr, ip, sizeof(ip))) return;
 
   if(grouped) printf("  ");
   printf("%s %s/%d\n", (af==AF_INET)?"IPv4":"IPv6", ip, netmask_prefix(e->ifa_netmask));
 }
 
 static void run_show(const char *filter, int show_all){
   struct ifaddrs *list=NULL;
 
   if(getifaddrs(&list)==-1){ perror("getifaddrs"); exit(1); }
 
   const char *prev=NULL;
   for(struct ifaddrs *cur=list; cur; cur=cur->ifa_next){
     if(!cur->ifa_addr) continue;
 
     int af = cur->ifa_addr->sa_family;
     if(af!=AF_INET && af!=AF_INET6) continue;
 
     if(filter && strcmp(cur->ifa_name, filter)!=0) continue;
 
     if(show_all && (!prev || strcmp(prev, cur->ifa_name)!=0)){
       printf("%s:\n", cur->ifa_name);
       prev = cur->ifa_name;
     }
 
     print_ip_line(cur, show_all);
   }
 
   freeifaddrs(list);
 }
 
 int main(int argc, char *argv[]){
   int opt, show_all=0; const char *ifname=NULL;
 
   while((opt=getopt(argc, argv, "ai:"))!=-1){
     if(opt=='a') show_all=1;
     else if(opt=='i') ifname=optarg;
     else { fprintf(stderr,"Usage: %s -a | %s -i <ifname>\n", argv[0], argv[0]); return 2; }
   }
 
 
   if((show_all && ifname) || (!show_all && !ifname)){
     fprintf(stderr,"Usage: %s -a | %s -i <ifname>\n", argv[0], argv[0]);
     return 2;
   }
 
   run_show(ifname, show_all);
   return 0;
 }
 
```

On utilise une Debian 13 pour développer, et une Debian 13 pour effectuer les tests.

---

### Explications

#### Fonction `bitcount8` : compter les bits à 1

```c
static int bitcount8(unsigned char v){ 
  int n=0; 
  while(v){ 
    n += v&1u; 
    v >>= 1u; 
  } 
  return n; 
}
```

- Parcourt les bits de `v` (octet) et additionne ceux qui valent 1.
- Sert à calculer `/prefix` à partir d’un masque réseau.

------

#### Fonction `netmask_prefix` : netmask → longueur de préfixe `/p`

```c
static int netmask_prefix(const struct sockaddr *mask){
  if(!mask) return 0;
```

- Sécurité : si pas de netmask → préfixe `0`.

  

**Cas IPv4**

```c
if(mask->sa_family == AF_INET){
    const unsigned char *m =
      (const unsigned char*)&((const struct sockaddr_in*)mask)->sin_addr;
    return bitcount8(m[0]) + bitcount8(m[1]) + bitcount8(m[2]) + bitcount8(m[3]);
  }
```

- Le masque IPv4 fait 4 octets : on additionne les bits à 1.
- Exemple : `255.255.255.0` → `8+8+8+0 = 24`.



**Cas IPv6**

```bash
if(mask->sa_family == AF_INET6){
    const unsigned char *m =
      (const unsigned char*)&((const struct sockaddr_in6*)mask)->sin6_addr;
    int p=0; for(int i=0;i<16;i++) p += bitcount8(m[i]); return p;
  }
```

- Le masque IPv6 fait 16 octets : on additionne les bits à 1 sur les 16 octets.
- Exemple courant : `/64`.



**Autres familles**

```c
return 0;
}
```

- Pour toute autre famille : `0`.



------

#### Fonction `print_ip_line` : formatage et affichage d’une adresse

```c
static void print_ip_line(struct ifaddrs *e, int grouped){
  int af = e->ifa_addr->sa_family;
  char ip[INET6_ADDRSTRLEN];
```

- `af` = famille d’adresse (`AF_INET` ou `AF_INET6`)
- `ip` = buffer texte pour l’adresse



**Récupération du pointeur vers l’adresse**

```c
void *addr = (af==AF_INET)
    ? (void*)&((struct sockaddr_in*)e->ifa_addr)->sin_addr
    : (void*)&((struct sockaddr_in6*)e->ifa_addr)->sin6_addr;
```

- IPv4 → `sin_addr`
- IPv6 → `sin6_addr`



**Conversion binaire → texte**

```
  if(!inet_ntop(af, addr, ip, sizeof(ip))) return;
```

- `inet_ntop()` écrit l’adresse lisible dans `ip`, sinon on ne print rien.



**Affichage final**

```c
if(grouped) printf("  ");
  printf("%s %s/%d\n", (af==AF_INET)?"IPv4":"IPv6", ip, netmask_prefix(e->ifa_netmask));
}
```

- Indente si mode `-a`.
- Affiche `IPv4/IPv6 adresse/prefixe` en utilisant `ifa_netmask` pour le `/p`.



------

#### Fonction `run_show` : boucle principale (récupération, filtrage, regroupement)

```c
static void run_show(const char *filter, int show_all){
  struct ifaddrs *list=NULL;

  if(getifaddrs(&list)==-1){ perror("getifaddrs"); exit(1); }
```

- `getifaddrs` fournit une liste chaînée de toutes les interfaces et adresses.



**Regroupement en mode `-a`**

```c
const char *prev=NULL;
```

- `prev` mémorise le dernier nom d’interface affiché pour ne pas répéter `eth0:` à chaque ligne.



**Parcours de la liste**

```c
for(struct ifaddrs *cur=list; cur; cur=cur->ifa_next){
    if(!cur->ifa_addr) continue;
```

- Ignore les entrées sans adresse.



**Ne garder que IPv4/IPv6**

```c
int af = cur->ifa_addr->sa_family;
    if(af!=AF_INET && af!=AF_INET6) continue;
```

- On ignore les autres types (`AF_PACKET`, etc.).



**Filtre si option `-i`**

```c
if(filter && strcmp(cur->ifa_name, filter)!=0) continue;
```

- Si `filter` est défini, on ne garde que l’interface demandée.



**Afficher le nom d’interface une seule fois en `-a`**

```c
if(show_all && (!prev || strcmp(prev, cur->ifa_name)!=0)){
      printf("%s:\n", cur->ifa_name);
      prev = cur->ifa_name;
    }
```

- On affiche `ifname:` uniquement quand on change d’interface.



**Afficher la ligne IPv4/IPv6**

```c
print_ip_line(cur, show_all);
  }
```

- Délègue l’affichage à `print_ip_line`.



**Libération mémoire**

```c
freeifaddrs(list);
}
```

- Libère la liste allouée par `getifaddrs`.



------

#### Fonction `main` : parsing des options et validation

```c
int main(int argc, char *argv[]){
  int opt, show_all=0; const char *ifname=NULL;
```

- `show_all` : vaut 1 si `-a`
- `ifname` : vaut le nom après `-i`



**Parsing avec `getopt`**

```c
while((opt=getopt(argc, argv, "ai:"))!=-1){
    if(opt=='a') show_all=1;
    else if(opt=='i') ifname=optarg;
    else { fprintf(stderr,"Usage: %s -a | %s -i <ifname>\n", argv[0], argv[0]); return 2; }
  }
```

- `"ai:"` : `-a` sans argument, `-i` avec argument obligatoire.
- `optarg` contient l’argument de `-i`.



**Vérification des combinaisons**

```c
if((show_all && ifname) || (!show_all && !ifname)){
    fprintf(stderr,"Usage: %s -a | %s -i <ifname>\n", argv[0], argv[0]);
    return 2;
  }
```

- Interdit : `-a` et `-i` ensemble.
- Interdit : aucune option.
- OK : seulement `-a` ou seulement `-i <ifname>`.



**Exécution**

```c
run_show(ifname, show_all);
  return 0;
}
```

- Lance l’affichage :
  - `-a` → toutes les interfaces regroupées
  - `-i eth0` → seulement `eth0`

------

#### Format de sortie obtenu

Exemple en mode `-a` :

```c
eth0:
  IPv4 192.168.1.10/24
  IPv6 fe80::a00:27ff:fe12:3456/64
```

Exemple en mode `-i eth0` :

```c
IPv4 192.168.1.10/24 IPv6 fe80::a00:27ff:fe12:3456/64
```

---

### 4.2 Intégrer la commande *ifshow* à la machine virtuelle VyOS du mini-projet 1

#### Récupérer `ifshow.c` sur la machine VyOS

On lance un serveur http sur la machine de développement en étant sur le répertoire sur lequel se trouve le binaire.

```shell
python3 -m http.server 8000
```

On garde ce terminal ouvert.

On créé un répertoire tools et l'on récupère avec `curl` le binaire.

```shell
mkdir -p /config/scripts/tools
curl -L http://10.0.2.8:8000/ifshow -o /config/scripts/tools/ifshow
chmod 755 /config/scripts/tools/ifshow
```

---

#### Test de la commande

En se plaçant dans le répertoire `tools` on utilise la commande `ifshow`.

```shell
./ifshow -a
./ifshow -i eth0
```

---

#### Intégration de la commande

On créé un lien au boot via `vyos-postconfig-bootup/script`.

```shell
sudo tee /config/scripts/vyos-postconfig-bootup.script >/dev/null <<'EOF'
#!/bin/sh
ln -sf /config/scripts/tools/ifshow /usr/local/bin/ifshow
EOF

sudo chmod +x /config/scripts/vyos-postconfig-bootup.script
```

---

#### Test de l'intégration

```shell
sudo ln -sf /config/scrips/tools/ifshow /usr/local/bin/ifshow
ifshow -a
```

Tout fonctionne.

---

### 4.3 Intégrer la commande *ifshow* à la machine virtuelle Alpine Linux du mini-projet 2

#### Récupérer `ifshow.c` sur la machine Alpine

On lance le serveur http sur la machine de développement en étant sur le répertoire sur lequel se trouve le script.

```shell
python3 -m http.server 8000
```

On garde ce terminal ouvert.

Sur la machine alpine on récupère le fichier avec `wget`.

```shell
wget http://10.0.2.8:8000/ifshow.c -O /root/ifshow.c
```

---

#### Installer un compilateur sur Alpine Linux

```shell
apk add build-base
```

---

#### Compiler le script

```shell
gcc ifshow.c -o ifshow
```

---

 #### Test de la commande `ifshow`

```shell
./ifshow -a
./ifshow -i eth0
```

---

#### Intégration de la commande

```shell
install -m 0755 ifshow /usr/local/bin/ifshow
```

---

#### Test de l'intégration

On essaie d'utiliser la commande `ifshow`.

```shell
ifshow -a
```

Tout est bien fonctionnel. Nous pouvons passer à l'intégration sur MicroCore.

---

### 4.4 Intégrer la commande *ifshow* à la machine virtuelle MicroCore du mini-projet 3

#### Récupérer `ifshow.c` sur la machine MicroCore

On lance le serveur http sur la machine de développement en étant sur le répertoire sur lequel se trouve le script. 

```shell
python3 -m http.server 8000
```

On garde ce terminal ouvert

Sur la machine MicroCore on télécharge le fichier avec `wget`.

```shell
wget http://IP_MACHINE_DEV:8000/ifshow.c -O /home/tc/ifshow.c
```

---

#### Installer un compilateur sur MicroCore

On installe un environnement de compilation

```shell
tce-load -wi compiletc
```

---

#### Compiler

On compile en **gnu99** :

```shell
gcc -std=gnu99 -Wall -Wextra -O2 ifshow.c -o ifshow
```

---

#### Test de la commande `ifshow`

```shell
./ifshow -a
./ifshow -i eth0
```

---

#### Intégration de la commande 

On crée un répertoire que nous allons ajouter au **PATH**.

```shell
mkdir -p /home/tc/bin
cp /home/tc/ifshow /home/tc/bin/ifshow
chmod +x /home/tc/bin/ifshow
```


Ajout de `bin` au **PATH**.

```shell
echo 'export PATH=$PATH:/home/tc/bin' >> /home/tc/.profile
. /home/tc/.profile
ifshow -a
```

---

#### Test de l'intégration

```shell
ifshow -a
```

---

On finit par sauvegarder avec `filetool.sh -b`.
