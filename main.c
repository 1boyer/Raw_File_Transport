/*******************************
    RFT: Raw File Transport
       Lance L. Boyer 2009
********************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <getopt.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "network.c"
#include "exchange.c"

#define MODE_SEND 0xFFFF0000
#define MODE_CONN 0x0000FFFF
#define MODE_HOST 0
#define MODE_RECV 0

#define DEFAULT_PORT 2344

void usage(char *const s){
  printf("Usage: %s [-s [filename]/-r] [-c/-h] -p [portno] [hostname]\n", s);
  printf("Options:\n-c Connect to host\n-h Wait for connection\n-s Send [filename]\n-r Receive a file\n");
  exit(-1);
}

void host_init(int addrno, int portno, char *filename){
  struct sockaddr_in saddr, ans_addr;
  int wait_sock, ans_sock, len;

  memset(&saddr, 0, sizeof(saddr));
  saddr.sin_family = AF_INET;
  saddr.sin_port = portno;
  saddr.sin_addr.s_addr = addrno;

  if((wait_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1){
    fprintf(stderr, "Unable to open socket! Dying...\n");
    exit(-1);
  }

  if(bind(wait_sock, (struct sockaddr *) &saddr, sizeof(saddr)) == -1){
    fprintf(stderr, "Unable to bind socket! Dying...\n");
    exit(-1);
  }

  if(listen(wait_sock, 2) == -1){
    fprintf(stderr, "Unable to listen on socket! Dying...\n");
    exit(-1);
  }

  len = 0;
  if((ans_sock = accept(wait_sock, (struct sockaddr *) &ans_addr, &len)) == -1){
    fprintf(stderr, "Unable to accept connection! Dying... Errno (%d)\n", errno);
    exit(-1);
  }

  exchange(ans_sock, filename);
  close(ans_sock);
  close(wait_sock);
}

void conn_init(int addrno, int portno, char* filename){
  struct sockaddr_in saddr;
  int conn_sock, len;

  memset(&saddr, 0, sizeof(saddr));
  saddr.sin_family = AF_INET;
  saddr.sin_port = portno;
  saddr.sin_addr.s_addr = addrno;

  conn_sock = socket(AF_INET, SOCK_STREAM, 0);
  if(connect(conn_sock, (struct sockaddr *) &saddr, sizeof(saddr)) == -1){
    fprintf(stderr, "Error connecting! Dying... Errno (%d)\n", errno);
    exit(-1);
  }
  exchange(conn_sock, filename);
  close(conn_sock);
}

int main(int argc, char** argv){
  int opt;
  int addrno = 0;
  int portno = 0;
  int mode = 0;
  int result;
  char *filename = 0;

  while((opt = getopt(argc, argv, "chs:rp:")) != -1){
    switch(opt){
      case 'c':
        mode |= MODE_CONN;
        break;
      case 'h':
        mode &= ~MODE_CONN;
        break;
      case 's':
        mode |= MODE_SEND;
        filename = optarg;
        break;
      case 'r':
        mode &= ~MODE_SEND;
        break;
      case 'p':
        portno = htons(strtol(optarg, 0, 10));
        break;
      demfault:
        usage(argv[0]);
     }
  }

  if(optind < argc){
    if((addrno = getaddr( argv[optind] )) != -1);
    else{ usage(argv[0]); }
  }

  switch(mode & MODE_SEND){
    case MODE_RECV:
      filename = 0;
      break;
    case MODE_SEND:
      if(filename == 0) usage(argv[0]);
      break;
  }

  if(portno == 0) portno = htons(DEFAULT_PORT);

  switch(mode & MODE_CONN){
    case MODE_HOST:
      if(addrno == 0) addrno = INADDR_ANY;
      host_init(addrno, portno, filename);
      break;
    case MODE_CONN:
      if(addrno == 0) usage(argv[0]);
      conn_init(addrno, portno, filename);
      break;
  }
  return 0;
}
