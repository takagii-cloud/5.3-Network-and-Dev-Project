// ifshow.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

static int bitcount8(unsigned char v){
  int n=0;
  while(v){
    n += v & 1u;   // compte le bit courant
    v >>= 1u;      // passe au bit suivant
  }
  return n;
}

static int netmask_prefix(const struct sockaddr *mask){
  if(!mask) return 0; // si pas de masque

  if(mask->sa_family == AF_INET){
    const unsigned char *m = (const unsigned char*)&((const struct sockaddr_in*)mask)->sin_addr;
    // calcule /prefix IPv4 en comptant les bits à 1
    return bitcount8(m[0]) + bitcount8(m[1]) + bitcount8(m[2]) + bitcount8(m[3]);
  }

  if(mask->sa_family == AF_INET6){
    const unsigned char *m = (const unsigned char*)&((const struct sockaddr_in6*)mask)->sin6_addr;
    // calcule /prefix IPv6 en comptant les bits à 1
    int p=0;
    for(int i=0;i<16;i++) p += bitcount8(m[i]);
    return p;
  }

  return 0;
}

static void print_ip_line(struct ifaddrs *e, int grouped){
  int af = e->ifa_addr->sa_family;   // IPv4 ou IPv6
  char ip[INET6_ADDRSTRLEN];         // buffer pour l'adresse en texte

  // récupère l'adresse (IPv4 ou IPv6)
  void *addr = (af==AF_INET)
    ? (void*)&((struct sockaddr_in*)e->ifa_addr)->sin_addr
    : (void*)&((struct sockaddr_in6*)e->ifa_addr)->sin6_addr;

  // conversion binaire -> texte
  if(!inet_ntop(af, addr, ip, sizeof(ip))) return;

  if(grouped) printf("  "); // indentation en mode -a
  printf("%s %s/%d\n", (af==AF_INET)?"IPv4":"IPv6", ip, netmask_prefix(e->ifa_netmask));
}

/* 1. retirer static pour que l’agent puisse appeler cette fonction */
void run_show(const char *filter, int show_all){
  struct ifaddrs *list=NULL;

  // récupère la liste des interfaces/adresses
  if(getifaddrs(&list)==-1){ perror("getifaddrs"); exit(1); }

  const char *prev=NULL; // dernière interface affichée (mode -a)

  for(struct ifaddrs *cur=list; cur; cur=cur->ifa_next){
    if(!cur->ifa_addr) continue; // entrée sans adresse

    int af = cur->ifa_addr->sa_family;
    if(af!=AF_INET && af!=AF_INET6) continue; // seulement IPv4/IPv6

    if(filter && strcmp(cur->ifa_name, filter)!=0) continue; // filtre -i

    // affiche le nom de l'interface une seule fois en -a
    if(show_all && (!prev || strcmp(prev, cur->ifa_name)!=0)){
      printf("%s:\n", cur->ifa_name);
      prev = cur->ifa_name;
    }

    // affiche l'adresse de cette entrée
    print_ip_line(cur, show_all);
  }

  freeifaddrs(list); // libère la mémoire
}

/* 2. entourer main pour pouvoir inclure ifshow.c dans l’agent */
#ifndef IFSHOW_NO_MAIN
int main(int argc, char *argv[]){
  int opt, show_all=0;
  const char *ifname=NULL;

  // lit -a ou -i <ifname>
  while((opt=getopt(argc, argv, "ai:"))!=-1){
    if(opt=='a') show_all=1;
    else if(opt=='i') ifname=optarg;
    else { fprintf(stderr,"Usage: %s -a | %s -i <ifname>\n", argv[0], argv[0]); return 2; }
  }

  // refuse les cas invalides (aucune option, ou -a et -i ensemble)
  if((show_all && ifname) || (!show_all && !ifname)){
    fprintf(stderr,"Usage: %s -a | %s -i <ifname>\n", argv[0], argv[0]);
    return 2;
  }

  run_show(ifname, show_all); // exécute l'affichage
  return 0;
}
#endif
	