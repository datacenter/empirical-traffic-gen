#ifndef __client_h
#define __client_h

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include <ctype.h>

#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_READSIZE 1048576

// prototypes
void *run_iteration(void *ptr);
void *listen_connection(void *ptr);
void read_config();
void read_args(int argc, char *argv[]);
void set_iteration_variables();
void open_connections();
pthread_t *launch_threads();
void process_stats();
void run_iterations();
void cleanup();

int listenfd;

#endif
