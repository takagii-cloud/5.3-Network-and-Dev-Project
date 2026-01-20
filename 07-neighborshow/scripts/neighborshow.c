// neighborshow.c (UDP only, robust IPv4 broadcast + IPv6 ff02::1)
// Prints unique neighbor hostnames (no duplicates), requires neighborshowd on peers.

#define _GNU_SOURCE
#define _DEFAULT_SOURCE

#include <arpa/inet.h>
#include <errno.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define UDP_PORT 9091
#define MAX_LINE 512
#define MAX_HOST 256
#define MAX_NEIGH 512

static void get_my_hostname(char *out, size_t cap) {
  if (gethostname(out, cap) != 0) strncpy(out, "unknown", cap);
  out[cap - 1] = '\0';
  for (char *p = out; *p; p++) {
    if (*p == '.') { *p = '\0'; break; }
  }
}

static int add_unique(char hosts[][MAX_HOST], int *count, int cap, const char *h) {
  for (int i = 0; i < *count; i++) {
    if (strcmp(hosts[i], h) == 0) return 0;
  }
  if (*count >= cap) return -1;
  snprintf(hosts[*count], MAX_HOST, "%s", h);
  (*count)++;
  return 1;
}

static int make_udp_v4(void) {
  int s = socket(AF_INET, SOCK_DGRAM, 0);
  if (s < 0) return -1;
  int yes = 1;
  setsockopt(s, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(yes));
  return s;
}

static int make_udp_v6(void) {
  int s = socket(AF_INET6, SOCK_DGRAM, 0);
  if (s < 0) return -1;
  int v6only = 0;
  setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY, &v6only, sizeof(v6only));
  return s;
}

static void bind_ephemeral_v4(int s4) {
  struct sockaddr_in a;
  memset(&a, 0, sizeof(a));
  a.sin_family = AF_INET;
  a.sin_port = 0;
  a.sin_addr.s_addr = htonl(INADDR_ANY);
  (void)bind(s4, (struct sockaddr *)&a, sizeof(a));
}

static void bind_ephemeral_v6(int s6) {
  struct sockaddr_in6 a6;
  memset(&a6, 0, sizeof(a6));
  a6.sin6_family = AF_INET6;
  a6.sin6_port = 0;
  a6.sin6_addr = in6addr_any;
  (void)bind(s6, (struct sockaddr *)&a6, sizeof(a6));
  int v6only = 0;
  setsockopt(s6, IPPROTO_IPV6, IPV6_V6ONLY, &v6only, sizeof(v6only));
}

static void send_ipv4_bcast(int s4, struct in_addr bcast, const char *msg) {
  struct sockaddr_in dst;
  memset(&dst, 0, sizeof(dst));
  dst.sin_family = AF_INET;
  dst.sin_port = htons(UDP_PORT);
  dst.sin_addr = bcast;
  (void)sendto(s4, msg, strlen(msg), 0, (struct sockaddr *)&dst, sizeof(dst));
}

static void send_ipv6_allnodes(int s6, unsigned ifindex, const char *msg) {
  struct sockaddr_in6 dst;
  memset(&dst, 0, sizeof(dst));
  dst.sin6_family = AF_INET6;
  dst.sin6_port = htons(UDP_PORT);
  inet_pton(AF_INET6, "ff02::1", &dst.sin6_addr);
  dst.sin6_scope_id = ifindex;
  (void)sendto(s6, msg, strlen(msg), 0, (struct sockaddr *)&dst, sizeof(dst));
}

int main(void) {
  char myhost[MAX_HOST];
  get_my_hostname(myhost, sizeof(myhost));

  // nonce unique
  char nonce[64];
  snprintf(nonce, sizeof(nonce), "%ld-%ld", (long)getpid(), (long)time(NULL));

  char msg[MAX_LINE];
  snprintf(msg, sizeof(msg), "NSHOW? %s\n", nonce);

  int s4 = make_udp_v4();
  int s6 = make_udp_v6();
  if (s4 < 0 || s6 < 0) {
    perror("socket");
    if (s4 >= 0) close(s4);
    if (s6 >= 0) close(s6);
    return 1;
  }

  bind_ephemeral_v4(s4);
  bind_ephemeral_v6(s6);

  // 1) Always try global broadcast (very helpful in VMs)
  {
    struct in_addr gb;
    gb.s_addr = inet_addr("255.255.255.255");
    send_ipv4_bcast(s4, gb, msg);
  }

  // 2) Per-interface sends
  struct ifaddrs *ifa = NULL;
  if (getifaddrs(&ifa) != 0) {
    perror("getifaddrs");
    close(s4); close(s6);
    return 1;
  }

  for (struct ifaddrs *it = ifa; it; it = it->ifa_next) {
    if (!it->ifa_addr) continue;
    if (!(it->ifa_flags & IFF_UP)) continue;
    if (it->ifa_flags & IFF_LOOPBACK) continue;

    // IPv4: compute broadcast = ip | ~netmask
    if (it->ifa_addr->sa_family == AF_INET) {
      struct sockaddr_in *ip = (struct sockaddr_in *)it->ifa_addr;
      struct sockaddr_in *nm = (struct sockaddr_in *)it->ifa_netmask;
      if (!ip || !nm) continue;

      struct in_addr bcast;
      bcast.s_addr = ip->sin_addr.s_addr | ~(nm->sin_addr.s_addr);
      send_ipv4_bcast(s4, bcast, msg);
    }

    // IPv6: ff02::1 scope per interface
    if (it->ifa_addr->sa_family == AF_INET6) {
      unsigned idx = if_nametoindex(it->ifa_name);
      if (idx) send_ipv6_allnodes(s6, idx, msg);
    }
  }

  freeifaddrs(ifa);

  // Collect replies (increase to 1500 ms for VM safety)
  char found[MAX_NEIGH][MAX_HOST];
  int fcount = 0;

  struct timeval start, now;
  gettimeofday(&start, NULL);

  while (1) {
    gettimeofday(&now, NULL);
    long elapsed = (now.tv_sec - start.tv_sec) * 1000L +
                   (now.tv_usec - start.tv_usec) / 1000L;
    if (elapsed >= 1500) break;

    long remain = 1500 - elapsed;
    if (remain < 0) remain = 0;

    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(s4, &rfds);
    FD_SET(s6, &rfds);
    int maxfd = (s4 > s6) ? s4 : s6;

    struct timeval tv;
    tv.tv_sec = (int)(remain / 1000);
    tv.tv_usec = (int)((remain % 1000) * 1000);

    int rc = select(maxfd + 1, &rfds, NULL, NULL, &tv);
    if (rc < 0) {
      if (errno == EINTR) continue;
      break;
    }
    if (rc == 0) continue;

    for (int pass = 0; pass < 2; pass++) {
      int s = (pass == 0) ? s4 : s6;
      if (!FD_ISSET(s, &rfds)) continue;

      char buf[MAX_LINE];
      struct sockaddr_storage src;
      socklen_t slen = sizeof(src);
      ssize_t n = recvfrom(s, buf, sizeof(buf) - 1, 0, (struct sockaddr *)&src, &slen);
      if (n <= 0) continue;
      buf[n] = '\0';

      if (strncmp(buf, "NSHOW!", 6) != 0) continue;

      char r_nonce[128] = {0};
      char r_host[MAX_HOST] = {0};
      if (sscanf(buf, "NSHOW! %127s %255s", r_nonce, r_host) != 2) continue;
      if (strcmp(r_nonce, nonce) != 0) continue;
      if (strcmp(r_host, myhost) == 0) continue;

      (void)add_unique(found, &fcount, MAX_NEIGH, r_host);
    }
  }

  close(s4);
  close(s6);

  for (int i = 0; i < fcount; i++) {
    printf("%s\n", found[i]);
  }

  return 0;
}
