// ifnetshow.c
#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>

#define PORT_STR "9090"

static void usage(const char *p){
  fprintf(stderr, "Usage:\n");
  fprintf(stderr, "  %s -n <addr> -a\n", p);
  fprintf(stderr, "  %s -n <addr> -i <ifname>\n", p);
}

static int connect_to(const char *addr){
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family   = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  struct addrinfo *res = NULL;
  int rc = getaddrinfo(addr, PORT_STR, &hints, &res);
  if(rc != 0){
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc));
    return -1;
  }

  int s = -1;
  for(struct addrinfo *p = res; p; p = p->ai_next){
    s = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if(s < 0) continue;
    if(connect(s, p->ai_addr, p->ai_addrlen) == 0){
      freeaddrinfo(res);
      return s;
    }
    close(s);
    s = -1;
  }

  freeaddrinfo(res);
  return -1;
}

int main(int argc, char **argv){
  int opt, show_all = 0;
  const char *addr = NULL;
  const char *ifname = NULL;

  while((opt = getopt(argc, argv, "n:ai:")) != -1){
    if(opt == 'n') addr = optarg;
    else if(opt == 'a') show_all = 1;
    else if(opt == 'i') ifname = optarg;
    else { usage(argv[0]); return 2; }
  }

  if(!addr || (show_all && ifname) || (!show_all && !ifname)){
    usage(argv[0]);
    return 2;
  }

  int s = connect_to(addr);
  if(s < 0){
    perror("connect");
    return 1;
  }

  char req[256];
  if(show_all) snprintf(req, sizeof(req), "ALL\n");
  else snprintf(req, sizeof(req), "IF %s\n", ifname);

  if(send(s, req, strlen(req), 0) < 0){
    perror("send");
    close(s);
    return 1;
  }

  // Lire jusqu’à EOF
  char buf[2048];
  while(1){
    ssize_t r = recv(s, buf, sizeof(buf), 0);
    if(r == 0) break;
    if(r < 0){
      if(errno == EINTR) continue;
      perror("recv");
      close(s);
      return 1;
    }
    fwrite(buf, 1, (size_t)r, stdout);
  }

  close(s);
  return 0;
}