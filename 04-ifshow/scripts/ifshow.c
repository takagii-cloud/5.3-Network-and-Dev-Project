#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

static int pop8(unsigned char x){ int c=0; while(x){ c+=x&1u; x>>=1u; } return c; }

static int prefixlen_from_netmask(const struct sockaddr *nm){
  if(!nm) return 0;
  if(nm->sa_family == AF_INET){
    const unsigned char *b = (const unsigned char*)&((const struct sockaddr_in*)nm)->sin_addr;
    return pop8(b[0])+pop8(b[1])+pop8(b[2])+pop8(b[3]);
  }
  if(nm->sa_family == AF_INET6){
    const unsigned char *b = (const unsigned char*)&((const struct sockaddr_in6*)nm)->sin6_addr;
    int s=0; for(int i=0;i<16;i++) s+=pop8(b[i]); return s;
  }
  return 0;
}

static void show_interface(const char *ifname_filter, int show_all){
  struct ifaddrs *ifaddr = NULL, *ifa = NULL;
  char buf[INET6_ADDRSTRLEN];
  const char *last = NULL;

  if(getifaddrs(&ifaddr) == -1){ perror("getifaddrs"); exit(1); }

  for(ifa = ifaddr; ifa; ifa = ifa->ifa_next){
    if(!ifa->ifa_addr) continue;

    int fam = ifa->ifa_addr->sa_family;
    if(fam != AF_INET && fam != AF_INET6) continue;

    if(ifname_filter && strcmp(ifa->ifa_name, ifname_filter) != 0) continue;

    if(show_all && (!last || strcmp(last, ifa->ifa_name) != 0)){
      printf("%s:\n", ifa->ifa_name);
      last = ifa->ifa_name;
    }

    void *src = (fam==AF_INET)
      ? (void*)&((struct sockaddr_in*)ifa->ifa_addr)->sin_addr
      : (void*)&((struct sockaddr_in6*)ifa->ifa_addr)->sin6_addr;

    if(!inet_ntop(fam, src, buf, sizeof(buf))) continue;

    int pfx = prefixlen_from_netmask(ifa->ifa_netmask);

    if(show_all) printf("  ");
    printf("%s %s/%d\n", (fam==AF_INET)?"IPv4":"IPv6", buf, pfx);
  }

  freeifaddrs(ifaddr);
}

int main(int argc, char *argv[]){
  int opt, all=0; const char *ifn=NULL;

  while((opt=getopt(argc, argv, "ai:")) != -1){
    if(opt=='a') all=1;
    else if(opt=='i') ifn=optarg;
    else { fprintf(stderr,"Usage: %s -a | %s -i <ifname>\n", argv[0], argv[0]); return 2; }
  }
  if((all && ifn) || (!all && !ifn)){
    fprintf(stderr,"Usage: %s -a | %s -i <ifname>\n", argv[0], argv[0]);
    return 2;
  }

  show_interface(ifn, all);
  return 0;
}