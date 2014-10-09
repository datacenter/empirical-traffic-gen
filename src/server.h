#ifndef __server_h
#define __server_h

//#define DEBUG

/* prototypes */ 
void handle_connection(int sockfd, const struct sockaddr_in *cliaddr);
void read_args(int argc, char*argv[]);

#endif

