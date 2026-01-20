// neighborshowd.c (UDP only, robust IPv4 + IPv6)
// Listens on UDP/9091 on BOTH IPv4 and IPv6, replies "NSHOW! <nonce> <hostname>".

#define _GNU_SOURCE
#define _DEFAULT_SOURCE

#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define UDP_PORT_STR "9091"
#define MAX_LINE 512
#define MAX_HOST 256

static void get_my_hostname(char *out, size_t cap) {
  if (gethostname(out, cap) != 0) strncpy(out, "unknown", cap);
  out[cap - 1] = '\0';
  for (char *p = out; *p; p++) {
    if (*p == '.') { *p = '\0'; break; }
  }
}

static int make_udp_server(int family) {
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = family;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;

  struct addrinfo *res = NULL;
  int rc = getaddrinfo(NULL, UDP_PORT_STR, &hints, &res);
  if (rc != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc));
    return -1;
  }

  int s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (s < 0) { freeaddrinfo(res); return -1; }

  int yes = 1;
  setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

  if (bind(s, res->ai_addr, res->ai_addrlen) < 0) {
    close(s);
    freeaddrinfo(res);
    return -1;
  }

  freeaddrinfo(res);
  return s;
}

static void handle_packet(int s, const char *myhost) {
  char buf[MAX_LINE];
  struct sockaddr_storage src;
  socklen_t slen = sizeof(src);

  ssize_t n = recvfrom(s, buf, sizeof(buf) - 1, 0, (struct sockaddr *)&src, &slen);
  if (n < 0) return;
  buf[n] = '\0';

  if (strncmp(buf, "NSHOW?", 6) != 0) return;

  // parse nonce after "NSHOW?"
  char nonce[128] = {0};
  const char *p = buf + 6;
  while (*p == ' ' || *p == '\t') p++;
  size_t i = 0;
  while (*p && *p != '\n' && *p != '\r' && i + 1 < sizeof(nonce)) {
    nonce[i++] = *p++;
  }
  nonce[i] = '\0';
  if (i == 0) return;

  char reply[MAX_LINE];
  snprintf(reply, sizeof(reply), "NSHOW! %s %s\n", nonce, myhost);
  (void)sendto(s, reply, strlen(reply), 0, (struct sockaddr *)&src, slen);
}

int main(void) {
  signal(SIGPIPE, SIG_IGN);

  char myhost[MAX_HOST];
  get_my_hostname(myhost, sizeof(myhost));

  int s4 = make_udp_server(AF_INET);
  int s6 = make_udp_server(AF_INET6);

  if (s4 < 0 && s6 < 0) {
    perror("neighborshowd: bind");
    return 1;
  }

  fprintf(stderr, "neighborshowd: listening UDP %s (hostname=%s)\n", UDP_PORT_STR, myhost);

  while (1) {
    fd_set rfds;
    FD_ZERO(&rfds);
    int maxfd = -1;

    if (s4 >= 0) { FD_SET(s4, &rfds); if (s4 > maxfd) maxfd = s4; }
    if (s6 >= 0) { FD_SET(s6, &rfds); if (s6 > maxfd) maxfd = s6; }

    int rc = select(maxfd + 1, &rfds, NULL, NULL, NULL);
    if (rc < 0) {
      if (errno == EINTR) continue;
      perror("select");
      break;
    }

    if (s4 >= 0 && FD_ISSET(s4, &rfds)) handle_packet(s4, myhost);
    if (s6 >= 0 && FD_ISSET(s6, &rfds)) handle_packet(s6, myhost);
  }

  if (s4 >= 0) close(s4);
  if (s6 >= 0) close(s6);
  return 0;
}
