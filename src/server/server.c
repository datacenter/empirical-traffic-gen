#include "server.h"
#include <fcntl.h>
#include "sys/sendfile.h"
#include <sys/stat.h>
#include <sys/time.h>
#include <netinet/tcp.h>

char *file_list[] = { "FILE_1KB", "FILE_10KB", "FILE_100KB", "FILE_1MB", "FILE_10MB", "FILE_100MB", "FILE_1GB" };

// prototypes                                                
void getDataFromTheClient(int);
void read_args(int argc, char*argv[]);

int sendFromMemory;
int serverPort;

int main (int argc, char *argv[]) {
  socklen_t len;
  struct sockaddr_in servaddr;
  struct sockaddr_in cliaddr;
  int sock_opt = 1;
  
  // read command line arguments
  read_args(argc, argv);
  
  // initialize socket
  listenfd = socket(AF_INET, SOCK_STREAM, 0);

  setsockopt(listenfd, IPPROTO_TCP, TCP_NODELAY, &sock_opt, sizeof(sock_opt));

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(serverPort);
  bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
  listen(listenfd, 20);

  if (sendFromMemory)
    printf("TCP test application server started... mode: send-from-memory\n");
  else
    printf("TCP test application server started... mode: send-from-disk\n");
  
  while(1) {
    // wait for connections                                                                                    
    len = sizeof(cliaddr);
    int sockfd;
    printf("Listening port %d!\n", serverPort);
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
  uint n;
  uint total = 0;
  char buf[50];
  int fd;
  struct stat stat_buf;
  uint f_index;
  uint f_size;
  uint file_id;
  char filebuf[MAX_WRITE];

  printf("%d - Connection established!\n", sockfd);
  while(1) {
    memset(buf, 0, 20);
    n = read(sockfd, buf, 2 * sizeof(uint));
    
    if (n <= 0)
      break;
    
    memcpy(&f_index, buf, sizeof(uint));
    memcpy(&file_id, buf + sizeof(uint), sizeof(uint));

    if (sendFromMemory) 
      f_size = file_id;
    else {      
      fd = open(file_list[file_id], O_RDONLY);
      fstat(fd, &stat_buf);
      f_size = stat_buf.st_size;
    }

    printf("File request: index: %u size: %d\n", f_index, f_size);
    
    // send meta data (f_index and f_size)
    memcpy(buf + sizeof(uint), &f_size, sizeof(uint));
    write(sockfd, buf, 2 * sizeof(uint));
    
    // send file
    if (sendFromMemory) {
      uint total = f_size;
      do {
	uint bytes_to_send;
	if (total > MAX_WRITE)
	  bytes_to_send = MAX_WRITE;
	else
	  bytes_to_send = total;
	uint bytes_sent = write(sockfd, filebuf, bytes_to_send);
	//printf("bytes_sent = %d\n", bytes_sent);

	if (bytes_sent <= 0) {
	  printf("failed to write...\n");
	  exit(1);
	}
	
	total -= bytes_sent;
      } while (total > 0);
      
    } else {
      sendfile(sockfd, fd, 0, f_size);
      close(fd);
    }
  }

  printf("\n%d - Connection closed! Bytes read: %u\n", sockfd, total);
  close(sockfd);
}


void read_args(int argc, char*argv[]) {
  // default values
  sendFromMemory = 1;
  serverPort = 5000;

  int i = 1;
  while (i < argc) {
    if (strcmp(argv[i], "-d") == 0) {
      sendFromMemory = 0;
      i++;
    } else if (strcmp(argv[i], "-p") == 0) {
      serverPort = atoi(argv[i+1]);
      i += 2;
    } else {
      printf("invalid option: %s\n", argv[i]);
      printf("usage: server [options]\n");
      printf("options:\n");
      printf("-d                         send from disk\n");
      printf("-p <value>                 port number (default 5000)\n");
      exit(1);
    }
  }
}
