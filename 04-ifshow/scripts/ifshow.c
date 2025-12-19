// ifshow.c
// gcc -Wall -Wextra -O2 ifshow.c -o ifshow.out
#include <errno.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

static void usage(const char *p) {
    fprintf(stderr,
            "Usage:\n"
            "  %s -a            Affiche toutes les interfaces + prefixes IPv4/IPv6\n"
            "  %s -i <ifname>   Affiche les prefixes IPv4/IPv6 de l'interface <ifname>\n",
            p, p);
}

static int popcnt8(unsigned char x) {
    int c = 0;
    while (x) {
        c += (x & 1u);
        x >>= 1u;
    }
    return c;
}

static int ipv4_prefixlen(struct in_addr mask) {
    const unsigned char *b = (const unsigned char *)&mask;
    return popcnt8(b[0]) + popcnt8(b[1]) + popcnt8(b[2]) + popcnt8(b[3]);
}

static void print_ipv4(struct in_addr a) {
    unsigned int x = ntohl(a.s_addr);
    printf("%u.%u.%u.%u",
           (x >> 24) & 255u,
           (x >> 16) & 255u,
           (x >> 8) & 255u,
           x & 255u);
}

static void print_ipv6_from_hex32(const char hex32[33]) {
    // Affiche en 8 groupes de 4 hex: xxxx:xxxx:... (pas de compression ::)
    for (int g = 0; g < 8; g++) {
        if (g) putchar(':');
        for (int i = 0; i < 4; i++) putchar(hex32[g * 4 + i]);
    }
}

static void show_ipv4_ioctl(int fd, const char *ifname, int indent) {
    struct ifreq r;
    memset(&r, 0, sizeof(r));
    strncpy(r.ifr_name, ifname, IFNAMSIZ - 1);

    if (ioctl(fd, SIOCGIFADDR, &r) != 0) {
        // pas d'IPv4 (ou pas de droits) => on ne sort rien
        return;
    }
    if (r.ifr_addr.sa_family != AF_INET) return;

    struct in_addr addr = ((struct sockaddr_in *)&r.ifr_addr)->sin_addr;

    struct ifreq m;
    memset(&m, 0, sizeof(m));
    strncpy(m.ifr_name, ifname, IFNAMSIZ - 1);

    int pfx = 0;
    if (ioctl(fd, SIOCGIFNETMASK, &m) == 0 && m.ifr_netmask.sa_family == AF_INET) {
        struct in_addr mask = ((struct sockaddr_in *)&m.ifr_netmask)->sin_addr;
        pfx = ipv4_prefixlen(mask);
    }

    if (indent) printf("  ");
    printf("IPv4 ");
    print_ipv4(addr);
    printf("/%d\n", pfx);
}

static void show_ipv6_from_proc(const char *ifname, int indent) {
    FILE *f = fopen("/proc/net/if_inet6", "r");
    if (!f) {
        // Si /proc pas monté ou IPv6 désactivé, on ne bloque pas le programme
        return;
    }

    char addr_hex[33];
    char name[IFNAMSIZ];
    unsigned int ifidx, plen, scope, flags;

    while (fscanf(f, "%32s %x %x %x %x %15s",
                  addr_hex, &ifidx, &plen, &scope, &flags, name) == 6) {
        if (strcmp(name, ifname) != 0) continue;

        if (indent) printf("  ");
        printf("IPv6 ");
        print_ipv6_from_hex32(addr_hex);
        printf("/%u\n", plen);
    }

    fclose(f);
}

static char *trim_left(char *s) {
    while (*s == ' ' || *s == '\t') s++;
    return s;
}

static void chomp(char *s) {
    size_t n = strlen(s);
    while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r')) {
        s[n - 1] = '\0';
        n--;
    }
}

static void list_all_interfaces_and_print(int fd) {
    FILE *f = fopen("/proc/net/dev", "r");
    if (!f) {
        perror("fopen(/proc/net/dev)");
        return;
    }

    char line[512];

    // Skip 2 header lines
    if (!fgets(line, sizeof(line), f)) { fclose(f); return; }
    if (!fgets(line, sizeof(line), f)) { fclose(f); return; }

    while (fgets(line, sizeof(line), f)) {
        chomp(line);
        char *p = trim_left(line);

        // interface name is before ':'
        char *colon = strchr(p, ':');
        if (!colon) continue;
        *colon = '\0';

        char ifname[IFNAMSIZ];
        strncpy(ifname, p, IFNAMSIZ - 1);
        ifname[IFNAMSIZ - 1] = '\0';

        printf("%s:\n", ifname);
        show_ipv4_ioctl(fd, ifname, 1);
        show_ipv6_from_proc(ifname, 1);
    }

    fclose(f);
}

int main(int argc, char **argv) {
    int opt;
    int mode_all = 0;
    const char *target = NULL;

    while ((opt = getopt(argc, argv, "ai:")) != -1) {
        switch (opt) {
        case 'a':
            mode_all = 1;
            break;
        case 'i':
            target = optarg;
            break;
        default:
            usage(argv[0]);
            return 2;
        }
    }

    if ((mode_all && target) || (!mode_all && !target)) {
        usage(argv[0]);
        return 2;
    }

    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        perror("socket(AF_INET)");
        return 1;
    }

    if (mode_all) {
        list_all_interfaces_and_print(fd);
    } else {
        // -i <ifname>
        show_ipv4_ioctl(fd, target, 0);
        show_ipv6_from_proc(target, 0);
    }

    close(fd);
    return 0;
}
