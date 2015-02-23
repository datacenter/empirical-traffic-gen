#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "common.h"
#include "server.h"

#define MAX_WRITE 104857600  // 100MB

/* Global variables */
int serverPort;
char flowbuf[MAX_WRITE];

int main (int argc, char *argv[]) {
  int listenfd;
  socklen_t len;
  struct sockaddr_in servaddr;
  struct sockaddr_in cliaddr;
  int sock_opt = 1;
  
  /* read command line arguments */
  read_args(argc, argv);
  
  /* initialize socket */
  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if (listenfd < 0)
    error("ERROR opening socket");

  if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &sock_opt, sizeof(sock_opt)) < 0)
    error("ERROR setting SO_REUSERADDR option");
  if (setsockopt(listenfd, IPPROTO_TCP, TCP_NODELAY, &sock_opt, sizeof(sock_opt)) < 0)
    error("ERROR setting TCP_NODELAY option");

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(serverPort);
  
  if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    error("ERROR on bind");
  
  if (listen(listenfd, 20) < 0)
    error("ERROR on listen");
  
  printf("Dynamic traffic generator application server started...\n");
  printf("Listening port: %d\n", serverPort);

  while(1) {
    /* wait for connections */ 
    int sockfd;
    len = sizeof(cliaddr);
    sockfd = accept(listenfd, (struct sockaddr *) &cliaddr, &len);
    if (sockfd < 0)
      error("ERROR on accept");

    pid_t pid = fork();
    if (pid < 0)
      error("ERROR on fork");

    if (pid == 0) {
      /* child process */
      if (close(listenfd) < 0)
	error("child: ERROR on close");
      handle_connection(sockfd, (const struct sockaddr_in *) &cliaddr);
      break;
    }
    else {
      /* parent process */
      if (close(sockfd) < 0)
	error("parent: ERROR on close");
    }
  }

  return 0;
}

/* 
 * Handles requests for an established connection. Each request is initiated
 * by the client with a small message providing meta-data for the request, 
 * specifically, a flow index and size. The server echoes the meta-data, and
 * subsequently sends a flow of the requested size to the client.
 */ 
void handle_connection(int sockfd, const struct sockaddr_in *cliaddr) {
  uint f_index;
  uint f_size;
  uint meta_data_size = 2 * sizeof(uint);
  char buf[16]; /* buffer to hold meta data */
  char clistr[INET_ADDRSTRLEN];

  if (inet_ntop(AF_INET, &(cliaddr->sin_addr), clistr, INET_ADDRSTRLEN) == NULL)
    error("ERROR on inet_ntop");

  printf("Connection established to %s (sockfd = %d)!\n", clistr, sockfd);

  while(1) {
    /* read meta-data */
    if (read_exact(sockfd, buf, meta_data_size, 16, false) 
	!= meta_data_size)
      break;

    /* extract flow index and size */
    memcpy(&f_index, buf, sizeof(uint));
    memcpy(&f_size, buf + sizeof(uint), sizeof(uint));

#ifdef DEBUG
    printf("Flow request: index: %u size: %d\n", f_index, f_size);
#endif
    
    /* echo meta-data (f_index and f_size) */
    if (write_exact(sockfd, buf, meta_data_size, MAX_WRITE, false) 
	!= meta_data_size)
      break;

    /* send flow of f_size bytes */
    if (write_exact(sockfd, flowbuf, f_size, MAX_WRITE, true) 
	!= f_size)
      break;
  }

  /* close_connection */
  close(sockfd);
  printf("Connection to %s closed (sockfd = %d)!\n", clistr, sockfd);
}

/*
 * Read command line arguments. 
 */
void read_args(int argc, char*argv[]) {
  /* default values */
  serverPort = 5000;

  int i = 1;
  while (i < argc) {
    if (strcmp(argv[i], "-p") == 0) {
      serverPort = atoi(argv[i+1]);
      i += 2;
    } else if (strcmp(argv[i], "-h") == 0) {
      print_usage();
      exit(EXIT_FAILURE);
    } else {
      printf("invalid option: %s\n", argv[i]);
      print_usage();
      exit(EXIT_FAILURE);
    }
  }
}

/*
 * Print usage.
 */
void print_usage() {
  printf("usage: server [options]\n");
  printf("options:\n");
  printf("-p <value>                 port number (default 5000)\n");
  printf("-h                           display usage information and quit\n");
}
