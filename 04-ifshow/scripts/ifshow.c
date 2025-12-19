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