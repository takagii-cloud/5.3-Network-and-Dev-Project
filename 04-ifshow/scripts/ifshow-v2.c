// ifshow.c (version simplifiée)
// gcc -Wall -Wextra -O2 ifshow.c -o ifshow.out
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

static void usage(const char *p) {
  fprintf(stderr, "Usage: %s -a | %s -i <ifname>\n", p, p);
}

static int pop8(unsigned char x){ int c=0; while(x){ c+=x&1u; x>>=1u; } return c; }

static int v4_pfx(struct in_addr m){
  unsigned char *b=(unsigned char*)&m;
  return pop8(b[0])+pop8(b[1])+pop8(b[2])+pop8(b[3]);
}

static void print_v4(struct in_addr a){
  unsigned int x = ntohl(a.s_addr);
  printf("%u.%u.%u.%u", (x>>24)&255u, (x>>16)&255u, (x>>8)&255u, x&255u);
}

static void print_v6_hex32(const char h[33]){
  for(int g=0; g<8; g++){
    if(g) putchar(':');
    for(int i=0;i<4;i++) putchar(h[g*4+i]);
  }
}

static void show_v4(int fd, const char *ifn, int indent){
  struct ifreq r; memset(&r,0,sizeof(r)); strncpy(r.ifr_name, ifn, IFNAMSIZ-1);
  if (ioctl(fd, SIOCGIFADDR, &r) != 0) return;
  if (r.ifr_addr.sa_family != AF_INET) return;

  struct in_addr addr = ((struct sockaddr_in*)&r.ifr_addr)->sin_addr;

  struct ifreq m; memset(&m,0,sizeof(m)); strncpy(m.ifr_name, ifn, IFNAMSIZ-1);
  int pfx = 0;
  if (ioctl(fd, SIOCGIFNETMASK, &m) == 0 && m.ifr_netmask.sa_family == AF_INET)
    pfx = v4_pfx(((struct sockaddr_in*)&m.ifr_netmask)->sin_addr);

  if(indent) printf("  ");
  printf("IPv4 "); print_v4(addr); printf("/%d\n", pfx);
}

static void show_v6(const char *ifn, int indent){
  FILE *f = fopen("/proc/net/if_inet6","r");
  if(!f) return;

  char hex[33], name[IFNAMSIZ];
  unsigned int ifidx, plen, scope, flags;

  while (fscanf(f, "%32s %x %x %x %x %15s", hex, &ifidx, &plen, &scope, &flags, name) == 6){
    if(strcmp(name, ifn)) continue;
    if(indent) printf("  ");
    printf("IPv6 "); print_v6_hex32(hex); printf("/%u\n", plen);
  }
  fclose(f);
}

static int next_ifname_from_netdev(FILE *f, char out[IFNAMSIZ]){
  char line[512];
  while (fgets(line, sizeof(line), f)){
    char *p = line;
    while(*p==' '||*p=='\t') p++;
    char *c = strchr(p, ':');
    if(!c) continue;
    *c = '\0';
    strncpy(out, p, IFNAMSIZ-1);
    out[IFNAMSIZ-1]='\0';
    return 1;
  }
  return 0;
}

int main(int argc, char **argv){
  int opt, all=0; const char *ifn=NULL;
  while((opt=getopt(argc, argv, "ai:"))!=-1){
    if(opt=='a') all=1;
    else if(opt=='i') ifn=optarg;
    else { usage(argv[0]); return 2; }
  }
  if ((all && ifn) || (!all && !ifn)) { usage(argv[0]); return 2; }

  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  if(fd<0){ perror("socket"); return 1; }

  if(all){
    FILE *f = fopen("/proc/net/dev","r");
    if(!f){ perror("fopen"); close(fd); return 1; }
    char tmp[512];
    fgets(tmp,sizeof(tmp),f); // header
    fgets(tmp,sizeof(tmp),f); // header

    char name[IFNAMSIZ];
    while(next_ifname_from_netdev(f, name)){
      printf("%s:\n", name);
      show_v4(fd, name, 1);
      show_v6(name, 1);
    }
    fclose(f);
  } else {
    show_v4(fd, ifn, 0);
    show_v6(ifn, 0);
  }

  close(fd);
  return 0;
}
