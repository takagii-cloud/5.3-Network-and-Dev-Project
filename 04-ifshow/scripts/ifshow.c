#include <stdio.h>
#include <string.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netinet/in.h>

static void usage(const char *p){
  fprintf(stderr, "Usage: %s -a | %s -i <ifname>\n", p, p);
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
    printf("%s %s\n", ifa->ifa_name, buf);
  }

  freeifaddrs(ifaddr);
  return 0;
}
