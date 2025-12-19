#include <stdio.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main(void){
  struct ifaddrs *ifaddr=NULL, *ifa=NULL;
  char buf[INET6_ADDRSTRLEN];

  if(getifaddrs(&ifaddr)==-1){ perror("getifaddrs"); return 1; }

  for(ifa=ifaddr; ifa; ifa=ifa->ifa_next){
    if(!ifa->ifa_addr) continue;
    int fam = ifa->ifa_addr->sa_family;
    if(fam!=AF_INET && fam!=AF_INET6) continue;

    void *src = (fam==AF_INET)
      ? (void*)&((struct sockaddr_in*)ifa->ifa_addr)->sin_addr
      : (void*)&((struct sockaddr_in6*)ifa->ifa_addr)->sin6_addr;

    if(!inet_ntop(fam, src, buf, sizeof(buf))) continue;
    printf("%s %s\n", ifa->ifa_name, buf);
  }

  freeifaddrs(ifaddr);
  return 0;
}
