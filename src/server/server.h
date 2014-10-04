#ifndef __server_h
#define __server_h

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_WRITE 104857600  // 100MB

//#define DEBUG

// prototypes                                                
void getDataFromTheClient(int sockfd, const struct sockaddr_in *cliaddr);
void read_args(int argc, char*argv[]);



#endif

