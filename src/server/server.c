#include "server.h"
#include <fcntl.h>
#include "sys/sendfile.h"
#include <sys/stat.h>
#include <sys/time.h>
#include <netinet/tcp.h>

// global variables
int serverPort;
int listenfd;
char filebuf[MAX_WRITE];

int main (int argc, char *argv[]) {
  socklen_t len;
  struct sockaddr_in servaddr;
  struct sockaddr_in cliaddr;
  int sock_opt = 1;
  
  // read command line arguments
  read_args(argc, argv);
  
  // initialize socket
  listenfd = socket(AF_INET, SOCK_STREAM, 0);

  setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &sock_opt, sizeof(sock_opt));
  setsockopt(listenfd, IPPROTO_TCP, TCP_NODELAY, &sock_opt, sizeof(sock_opt));

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(serverPort);
  
  int ret = bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
  if (ret < 0) {
    fprintf(stderr, "error in bind: %s\n", strerror(errno));
    exit(-1);
  }
  
  ret = listen(listenfd, 20);
  if (ret < 0) {
    fprintf(stderr,"error in listen: %s\n", strerror(errno));
    exit(-1);
  }
  
  printf("TCP test application server started...\n");
  printf("Listening port %d!\n", serverPort);

  while(1) {
    // wait for connections          
    int sockfd;
    len = sizeof(cliaddr);
    sockfd = accept(listenfd, (struct sockaddr *) &cliaddr, &len);
    
    pid_t pid = fork();

    // child process
    if (pid == 0) {
      close(listenfd);
      getDataFromTheClient(sockfd);
      break;
    }
    // parent process
    else {
      close(sockfd);
    }
  }

  return 0;
}

// listen client
void getDataFromTheClient(int sockfd) {
  uint f_index;
  uint f_size;
  char buf[50];

  printf("%d - Connection established!\n", sockfd);

  while(1) {
    // read request
    int n = read(sockfd, buf, 2 * sizeof(uint));
    if (n < 0) {
      perror("error in meta-data read");
      break;
    }
    
    memcpy(&f_index, buf, sizeof(uint));
    memcpy(&f_size, buf + sizeof(uint), sizeof(uint));

#ifdef DEBUG
    printf("File request: index: %u size: %d\n", f_index, f_size);
#endif 
    
    // send meta data (f_index and f_size)
    n = write(sockfd, buf, 2 * sizeof(uint));
    if (n < 0) {
      perror("error in meta-data write");
      break;
    }

    // send file
    uint total = f_size;
    do {
      uint bytes_to_send;
      if (total > MAX_WRITE)
	bytes_to_send = MAX_WRITE;
      else
	bytes_to_send = total;
      int bytes_sent = write(sockfd, filebuf, bytes_to_send);
      
      if (bytes_sent < 0) {
	perror("error in file write");
	break;
      }
      
      total -= bytes_sent;
    } while (total > 0); 
  }

  printf("\n%d - Connection closed!\n", sockfd);
  close(sockfd);
}

void read_args(int argc, char*argv[]) {
  // default values
  serverPort = 5000;

  int i = 1;
  while (i < argc) {
    if (strcmp(argv[i], "-p") == 0) {
      serverPort = atoi(argv[i+1]);
      i += 2;
    } else {
      printf("invalid option: %s\n", argv[i]);
      printf("usage: server [options]\n");
      printf("options:\n");
      printf("-p <value>                 port number (default 5000)\n");
      exit(1);
    }
  }
}
