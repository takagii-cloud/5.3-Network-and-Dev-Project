#include <stdio.h>
#include <string.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netinet/in.h>

static void usage(const char *p){
  fprintf(stderr, "Usage: %s -a | %s -i <ifname>\n", p, p);
}

static int pop8(unsigned char x){ int c=0; while(x){ c+=x&1u; x>>=1u; } return c; }

static int ipv4_prefixlen(const struct sockaddr *nm){
  if(!nm || nm->sa_family != AF_INET) return 0;
  const unsigned char *b =
    (const unsigned char*)&((const struct sockaddr_in*)nm)->sin_addr;
  return pop8(b[0]) + pop8(b[1]) + pop8(b[2]) + pop8(b[3]);
}

int main(int argc, char *argv[]){
  int all = 0;
  const char *filter = NULL;

  if(argc == 2 && strcmp(argv[1], "-a")==0){
    all = 1;
  } else if(argc == 3 && strcmp(argv[1], "-i")==0){
    filter = argv[2];
  } else {
    usage(argv[0]);
    return 2;
  }

  struct ifaddrs *ifaddr=NULL, *ifa=NULL;
  char buf[INET6_ADDRSTRLEN];

  if(getifaddrs(&ifaddr)==-1){ perror("getifaddrs"); return 1; }

  for(ifa=ifaddr; ifa; ifa=ifa->ifa_next){
    if(!ifa->ifa_addr) continue;
    int fam = ifa->ifa_addr->sa_family;
    if(fam!=AF_INET && fam!=AF_INET6) continue;

    if(!all && filter && strcmp(ifa->ifa_name, filter) != 0) continue;

    void *src = (fam==AF_INET)
      ? (void*)&((struct sockaddr_in*)ifa->ifa_addr)->sin_addr
      : (void*)&((struct sockaddr_in6*)ifa->ifa_addr)->sin6_addr;

    if(!inet_ntop(fam, src, buf, sizeof(buf))) continue;

    int pfx = (fam==AF_INET) ? ipv4_prefixlen(ifa->ifa_netmask) : 0;

    printf("%s %s %s/%d\n",
           ifa->ifa_name,
           (fam==AF_INET) ? "IPv4" : "IPv6",
           buf, pfx);
  }

  freeifaddrs(ifaddr);
  return 0;
}
