// ifnetshowd.c
#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

#define IFSHOW_NO_MAIN
#include "ifshow.c"

#define PORT_STR "9090"
#define LINE_MAX 256

static int read_line(int fd, char *buf, size_t cap){
  size_t i = 0;
  while(i + 1 < cap){
    char c;
    ssize_t r = recv(fd, &c, 1, 0);
    if(r == 0) break;              // EOF
    if(r < 0){
      if(errno == EINTR) continue;
      return -1;
    }
    if(c == '\n') break;
    buf[i++] = c;
  }
  buf[i] = '\0';
  return (int)i;
}

static int ifname_ok(const char *s){
  // simple: autorise lettres/chiffres/._- (évite injection et trucs bizarres)
  if(!s || !*s) return 0;
  for(const unsigned char *p=(const unsigned char*)s; *p; p++){
    if(!(isalnum(*p) || *p=='_' || *p=='-' || *p=='.')) return 0;
  }
  return 1;
}

static void send_err(int fd, const char *msg){
  (void)send(fd, msg, strlen(msg), 0);
}

static void handle_client(int cfd){
  char line[LINE_MAX];
  int n = read_line(cfd, line, sizeof(line));
  if(n <= 0){
    return;
  }

  // trim espaces
  while(n > 0 && (line[n-1] == '\r' || line[n-1] == ' ' || line[n-1] == '\t')) line[--n] = '\0';

  int mode_all = 0;
  const char *ifname = NULL;

  if(strcmp(line, "ALL") == 0){
    mode_all = 1;
  } else if(strncmp(line, "IF ", 3) == 0){
    ifname = line + 3;
    while(*ifname == ' ' || *ifname == '\t') ifname++;
    if(!ifname_ok(ifname)){
      send_err(cfd, "ERR bad-ifname\n");
      return;
    }
  } else {
    send_err(cfd, "ERR bad-request\n");
    return;
  }

  // Redirige stdout + stderr vers le client
  int old_out = dup(STDOUT_FILENO);
  int old_err = dup(STDERR_FILENO);
  if(old_out < 0 || old_err < 0){
    send_err(cfd, "ERR internal\n");
    return;
  }

  if(dup2(cfd, STDOUT_FILENO) < 0 || dup2(cfd, STDERR_FILENO) < 0){
    send_err(cfd, "ERR internal\n");
    close(old_out); close(old_err);
    return;
  }

  // Optionnel: rendre stdout non bufferisé pour que ça parte direct
  setvbuf(stdout, NULL, _IONBF, 0);
  setvbuf(stderr, NULL, _IONBF, 0);

  // Réutilisation directe de ton code ifshow
  if(mode_all){
    run_show(NULL, 1);
  } else {
    run_show(ifname, 0);
  }

  fflush(NULL);

  // Restore stdout/stderr
  dup2(old_out, STDOUT_FILENO);
  dup2(old_err, STDERR_FILENO);
  close(old_out);
  close(old_err);
}

int main(void){
  signal(SIGPIPE, SIG_IGN);

  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family   = AF_INET6;       // socket IPv6
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags    = AI_PASSIVE;

  struct addrinfo *res = NULL;
  int rc = getaddrinfo(NULL, PORT_STR, &hints, &res);
  if(rc != 0){
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc));
    return 1;
  }

  int s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if(s < 0){ perror("socket"); freeaddrinfo(res); return 1; }

  int yes = 1;
  setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

  // accepte IPv4-mapped aussi
  int v6only = 0;
  setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY, &v6only, sizeof(v6only));

  if(bind(s, res->ai_addr, res->ai_addrlen) < 0){
    perror("bind");
    freeaddrinfo(res);
    close(s);
    return 1;
  }
  freeaddrinfo(res);

  if(listen(s, 16) < 0){
    perror("listen");
    close(s);
    return 1;
  }

  fprintf(stderr, "ifnetshowd listening on port %s (TCP)\n", PORT_STR);

  while(1){
    struct sockaddr_storage ss;
    socklen_t slen = sizeof(ss);
    int cfd = accept(s, (struct sockaddr*)&ss, &slen);
    if(cfd < 0){
      if(errno == EINTR) continue;
      perror("accept");
      continue;
    }
    handle_client(cfd);
    close(cfd);
  }
}