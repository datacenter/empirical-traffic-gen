#ifndef __server_h
#define __server_h

#define MAX_WRITE 104857600  // 100MB

//#define DEBUG

// prototypes                                                
void handle_connection(int sockfd, const struct sockaddr_in *cliaddr);
void read_args(int argc, char*argv[]);
void error(const char *msg);

#endif

