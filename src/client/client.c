#include "client.h"
#include "sys/sendfile.h"
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <netinet/tcp.h>
#include <pthread.h>
//#include <sys/fcntl.h> 


// command line arguments
int serverPort;
char config_name[80];

// input parameters
int num_dest;
int *dest_port;
int *src_port;
char (*dest_addr)[20];

int num_files;
int *file_size;
int *file_prob;
int file_prob_total;

int num_fanouts;
int *fanout_size;
int *fanout_prob;
int fanout_prob_total;

int iter;
int period;

// per-iteration variables
int *iteration_fanout;
uint *iteration_findex;
int *iteration_file_size;
int *iteration_destination;
struct timeval *start_time;
struct timeval *stop_time;

uint *dest_file_count;

// sockets for communicating with each destination
int *sockets;            

// threading
pthread_mutex_t inc_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_t *threads;

int req_file_count;
int req_index;

                  
int main (int argc, char *argv[]) {

  // read command line arguments
  read_args(argc, argv);

  // read input configuration
  read_config();

  // set up per-iteration variables
  set_iteration_variables();

  // open connections to destination address/port combos
  open_connections();

  // launch threads
  threads = launch_threads();

  // run iterations
  run_iterations();

  // check for completion of all threads
  for (int i = 0; i < num_dest; i++) {
    pthread_join(threads[i], NULL);
  }
  printf("All iterations completed. Processing statistics...\n");

  // process statistics
  process_stats();

 
  printf("client terminated...\n");

  // clean up; especially free allocated memory
  cleanup();

  return 0;
}


void run_iterations() {
  if (period < 0) {
    int *i_index = malloc(sizeof(int));
    req_index = 0;
    *i_index = req_index;
    run_iteration(i_index);
  } 
  else {
    for (int i = 0; i < iter; i++) {
      int *i_index = malloc(sizeof(int));
      *i_index = i;
      run_iteration(i_index);
      usleep(period * 1000);
    }
  }
}

void cleanup() {  
  free(dest_port);
  free(src_port);
  free(dest_addr);
  free(sockets);
  free(threads);
  free(dest_file_count);
  
  free(file_size);
  free(file_prob);

  free(fanout_size);
  free(fanout_prob);

  free(iteration_fanout);
  free(iteration_file_size);
  free(iteration_destination);  
  free(stop_time);
  free(start_time);
}

void process_stats() {
  // stats for all iterations
  uint max_iter_usec = 0;
  uint min_iter_usec = 0;
  uint avg_iter_usec = 0;
  
  // stats per fanout
  uint *f_max_iter_usec = malloc(num_fanouts * sizeof(uint));
  uint *f_min_iter_usec = malloc(num_fanouts * sizeof(uint));
  uint *f_avg_iter_usec = malloc(num_fanouts * sizeof(uint));
  uint *f_iter_count = malloc(num_fanouts * sizeof(uint));
  
  for (int i = 0; i < num_fanouts; i++) {
    f_max_iter_usec[i] = 0;
    f_min_iter_usec[i] = 0;
    f_avg_iter_usec[i] = 0;
    f_iter_count[i] = 0;
  }
  
  // start per file size
  uint *f_max_file_usec = malloc(num_files * sizeof(uint));
  uint *f_min_file_usec = malloc(num_files * sizeof(uint));
  uint *f_avg_file_usec = malloc(num_files * sizeof(uint));
  uint64_t *f_var_file_usec = malloc(num_files * sizeof(uint64_t));
  uint *f_file_count = malloc(num_files * sizeof(uint));
  
  for (int i = 0; i < num_files; i++) {
    f_max_file_usec[i] = 0;
    f_min_file_usec[i] = 0;
    f_avg_file_usec[i] = 0;
    f_var_file_usec[i] = 0;
    f_file_count[i] = 0;
  }
  
  for (int i = 0; i < iter; i++) {
    int i_usec;
    int i_sec;
    struct timeval *i_start;
    struct timeval *i_stop;
    
    for (int j = 0; j < iteration_fanout[i]; j++) {
      uint index = i * num_dest + j;
      
      // find the start and stop time of the iteration
      if (j == 0) {
	i_start = &start_time[index];
	i_stop = &stop_time[index];
      }
      else {
	if (start_time[index].tv_sec < i_start->tv_sec || 
	    (start_time[index].tv_sec == i_start->tv_sec && start_time[index].tv_usec < i_start->tv_usec)) {
	  i_start = &start_time[index];
	}
	
	if (stop_time[index].tv_sec > i_stop->tv_sec || 
	    (stop_time[index].tv_sec == i_stop->tv_sec && stop_time[index].tv_usec > i_stop->tv_usec)) {
	  i_stop = &stop_time[index];
	}
      }
      
      // measure file transfer completion time
      int f_index;
      for (int k = 0; k < num_files; k++) {
	if (file_size[k] == iteration_file_size[index]) {
	  f_index = k;
	  break;
	}
      }
      int f_sec = stop_time[index].tv_sec - start_time[index].tv_sec;
      int f_usec = stop_time[index].tv_usec - start_time[index].tv_usec;      
      f_usec += (f_sec * 1000000);
      
      if (f_usec > f_max_file_usec[f_index]) 
	f_max_file_usec[f_index] = f_usec;
      
      if (f_usec < f_min_file_usec[f_index])
	f_min_file_usec[f_index] = f_usec;
      
      f_avg_file_usec[f_index] += f_usec;
      f_var_file_usec[f_index] += (((int64_t)f_usec) * f_usec);
      f_file_count[f_index]++;
    }
    
    // measure iteration completion time
    i_sec = i_stop->tv_sec - i_start->tv_sec;
    i_usec = i_stop->tv_usec - i_start->tv_usec;
    i_usec += (i_sec*1000000);
        
    if (i_usec > max_iter_usec)
      max_iter_usec = i_usec;
    
    if (i_usec < min_iter_usec)
      min_iter_usec = i_usec;
    
    avg_iter_usec += i_usec;
    
    // measure iteration completion time per fanout
    int f_index;
    for (int k = 0; k < num_fanouts; k++) {
      if (fanout_size[k] == iteration_fanout[i]) {
	f_index = k;
	break;
      }
    }
    
    if (i_usec > f_max_iter_usec[f_index]) 
      f_max_iter_usec[f_index] =i_usec;
    
    if (i_usec < f_min_iter_usec[f_index]) 
      f_min_iter_usec[f_index] = i_usec;
       
    f_avg_iter_usec[f_index] += i_usec;
    f_iter_count[f_index]++;
    
    printf("Iteration: %d, duration: %u usec\n", i, i_usec);
  }
  
  printf("=== Stats for fanout sizes ===\n");  
  for (int i = 0 ; i < num_fanouts; i++) {
    printf("Fanout: %d - count: %d\n", fanout_size[i], f_iter_count[i]);
    if (f_iter_count[i] > 0) {
      printf("Max duration : %u usec\n", f_max_iter_usec[i]);
      printf("Avg iteration: %u usec\n", f_avg_iter_usec[i] / f_iter_count[i]);
      printf("Min duration:  %u usec\n", f_min_iter_usec[i]);
    }
  }
  
  
  printf("=== Stats for file sizes ===\n");  
  for (int i = 0 ; i < num_files; i++) {
    printf("%d - count: %d\n", file_size[i], f_file_count[i]);
    if (f_file_count[i] > 0) {
      uint avg_val = f_avg_file_usec[i] / f_file_count[i];
      uint64_t var_val = f_var_file_usec[i] / f_file_count[i];
      var_val -= (((uint64_t)avg_val) * avg_val);
      printf("Max duration : %u usec\n", f_max_file_usec[i]);
      printf("Avg iteration: %u usec\n", avg_val);
      printf("Standard deviation: %u usec\n", (uint)sqrt(var_val));
      printf("Min duration: %u usec\n", f_min_file_usec[i]);
    }
  }
  
  printf("=== Stats for all iterations ===\n");
  printf("Max iteration: %u usec\n", max_iter_usec);
  printf("Avg iteration: %u usec\n", avg_iter_usec / iter);
  printf("Min iteration: %u usec\n", min_iter_usec);

}

pthread_t *launch_threads() {
  pthread_t *threads = malloc(num_dest * sizeof(pthread_t));
  
  for (int i = 0; i < num_dest; i++) {
    int *l_index = malloc(sizeof(int));
    *l_index = i;
    pthread_create(&threads[i], NULL, listen_connection, l_index);
  }

  return threads;
}

void open_connections() {
  int sock_opt = 1;
  int ret;

  printf("connect to servers\n");
  
  for (int i = 0; i < num_dest; i++) {
    struct sockaddr_in servaddr;
    int sock;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    
    // set socket options
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &sock_opt, sizeof(sock_opt));
    
    // bind to local port if provided; otherwise we use an ephemeral port
    if (src_port[i] > 0) {
      memset(&servaddr, 0, sizeof(servaddr));
      servaddr.sin_family = AF_INET;
      servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
      servaddr.sin_port = htons(src_port[i]);
      bind(sock, (struct sockaddr *) &servaddr, sizeof(servaddr));
    }      
    
    // connect to destination server (address/port)
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(dest_port[i]);
    inet_pton(AF_INET, dest_addr[i], &servaddr.sin_addr);
    
    ret = connect(sock, (struct sockaddr *) &servaddr, sizeof(servaddr));
    
    if (ret < 0) {
      perror("Unable to connect");
      sockets[i] = -1;
      exit(-1);
    }
    else {
      printf("Connected to %s on port %d\n", dest_addr[i], dest_port[i]);
      sockets[i] = sock;
    }
  }
}

void set_iteration_variables() {
  // generate fanout and files sizes for each iteration
  iteration_fanout = malloc(iter * sizeof(int));
  iteration_file_size = malloc(iter * num_dest * sizeof(int));
  iteration_destination = malloc(iter * num_dest * sizeof(int));
  stop_time = malloc(iter * num_dest * sizeof(struct timeval));
  start_time = malloc(iter * num_dest * sizeof(struct timeval));

  int *temp_list = malloc(num_dest * sizeof(int));
  int *dest_list = malloc(num_dest * sizeof(int));
  
  for (int i = 0; i < num_dest; i++) {
    temp_list[i] = i;
  }
  
  for (int i = 0; i < iter; i++) {
    int val = rand() % fanout_prob_total;
    // generate fanout size
    for (int j = 0; j < num_fanouts; j++) {
      if (val < fanout_prob[j]) {
	iteration_fanout[i] = fanout_size[j];
	break;
      }
      else {
	val -= fanout_prob[j];
      }
    }
    
    printf("%d - Fanout size: %d\n", i, iteration_fanout[i]);
    
    memcpy(dest_list, temp_list, num_dest * sizeof(int));
    int dest_count = num_dest;
    
    // generate file sizes and destination
    for (int j = 0; j < iteration_fanout[i]; j++) {
      // generate file size
      int val = rand() % file_prob_total;
      
      for (int k = 0; k < num_files; k++) {
	if (val < file_prob[k]) {
	  iteration_file_size[i * num_dest + j] = file_size[k];
	  break;
	}
	else {
	  val -= file_prob[k];
	}
      }
      
      // generate destination
      val = rand() % dest_count;
      iteration_destination[i * num_dest + j] = dest_list[val];
      dest_file_count[dest_list[val]]++;
      dest_count--;
      dest_list[val] = dest_list[dest_count];
      
      printf("%d, %d - Dest: %d, File: %d\n", i, j, iteration_destination[i * num_dest +j], iteration_file_size[i * num_dest + j]);
    }
  }

  free(temp_list);
  free(dest_list);
}

void read_args(int argc, char*argv[]) {
  // default values
  strcpy(config_name, "config");
  
  int i = 1;
  while (i < argc) {
    if (strcmp(argv[i], "-c") == 0) {
      strcpy(config_name, argv[i+1]);
      i += 2;
    } else {
      printf("invalid option: %s\n", argv[i]);
      printf("usage: server [options]\n");
      printf("options:\n");
      printf("-c <name>                  configuration file name\n");
      exit(1);
    }
  }
}


void read_config() {
  FILE *fd;

  printf("Reading file: %s\n", config_name);
  fd = fopen(config_name, "r");
  
  // destinations
  fscanf (fd, "destinations %d\n", &num_dest);
  printf("Number of destinations: %d\n", num_dest);
  
  dest_addr = malloc(num_dest * sizeof(char[20]));
  dest_port = malloc(num_dest * sizeof(int));
  src_port = malloc(num_dest * sizeof(int));
  sockets = malloc(num_dest * sizeof(int));
  dest_file_count = malloc(num_dest * sizeof(uint));
  
  for (int i = 0; i < num_dest; i++) {
    int tmp;
    fscanf(fd, "%d dest %s %d %d\n", &tmp, dest_addr[i], &dest_port[i], &src_port[i]);
    printf("%d\t- Dest: %s,\t%d - Src: %d\n", tmp, dest_addr[i], dest_port[i], src_port[i]);
    
    dest_file_count[i] = 0;
  }

  // file sizes
  fscanf(fd, "files %d\n", &num_files);
  printf("===\nNumber of files: %d\n", num_files);
  file_size = malloc(num_files * sizeof(int));
  file_prob = malloc(num_files * sizeof(int));
  file_prob_total = 0;
  
  for (int i = 0; i < num_files; i++) {
    int tmp;
    fscanf(fd, "%d file %d %d\n", &tmp, &file_size[i], &file_prob[i]);
    file_prob_total += file_prob[i];
    printf("%d\t- File: %d,\tProb: %d\n", tmp, file_size[i], file_prob[i]);
  }
  printf("Total file size prob: %d\n", file_prob_total);

  // fanouts
  fscanf(fd, "fanouts %d\n", &num_fanouts);
  printf("===\nNumber of fanouts: %d\n", num_fanouts);
  fanout_size = malloc(num_fanouts * sizeof(int));
  fanout_prob = malloc(num_fanouts * sizeof(int));
  fanout_prob_total = 0;

  for (int i = 0; i < num_fanouts; i++) {
    int tmp;
    fscanf(fd, "%d fanout %d %d\n", &tmp, &fanout_size[i], &fanout_prob[i]);
    fanout_prob_total += fanout_prob[i];
    printf("%d\t- Fanout: %d,\tProb: %d\n", tmp, fanout_size[i], fanout_prob[i]);
  }
  printf("Total fanout prob: %d\n", fanout_prob_total);
  
  fscanf(fd, "period %d\n", &period);
  printf("===\nPeriod: %d\n", period);

  fscanf(fd, "iterations %d\n", &iter);
  printf("===\nIterations: %d\n", iter);
  
  fclose(fd);
}


void *run_iteration(void *ptr) {
  int i = *((int *)ptr);
  struct timeval tstart, tstop;
  int usec;
  int sec;
  char buf[30];
   
  free(ptr);

  if (period < 0) {
    req_file_count = iteration_fanout[i];
  }

  printf("Iteration: %d, fanout: %d\n", i, iteration_fanout[i]);

  gettimeofday(&tstart, NULL);

  // send requests
  for (int j = 0; j < iteration_fanout[i]; j++) {
    //printf("Iteration: %d, Reqeust: %d..\n", i, j);
    uint index = i * num_dest + j;
    
    memcpy(buf, &index, sizeof(uint));
    memcpy(buf + sizeof(uint), &iteration_file_size[index], sizeof(uint));    
    
    gettimeofday(&start_time[index], NULL);
    
    write(sockets[iteration_destination[index]], buf, 2 * sizeof(uint));
  }
 
  gettimeofday(&tstop, NULL);
  sec = tstop.tv_sec - tstart.tv_sec;
  usec = tstop.tv_usec - tstart.tv_usec;
  
  if (usec < 0) {
    usec += 1000000;
    sec--;
  }
  
  usec += sec * 1000000;
    
  printf("Duration: %u sec, %u usec\n", sec, usec); 
  return NULL;
}

void *listen_connection(void *ptr) {
  int index = *((int *)ptr);
  free(ptr);

  int sock = sockets[index];
  char buf[MAX_READSIZE];
  int meta_read_size = 2 * sizeof(int);
  int file_count = 0;
  int n;

  while(file_count < dest_file_count[index]) {
    //printf("%d - wait for meta data, left: %d\n", index, (dest_file_count[index] - file_count));
    int total = 0;
    while (total < meta_read_size) {
      char readbuf[50];

      n = read(sock,readbuf, meta_read_size);
      memcpy(buf + total, readbuf, n);
      total += n;
    }
    if (n != meta_read_size) {
      printf("Read meta data size is incorrect!\n");
      exit(-1);
    }
    
    uint f_index;
    uint f_size;
    
    memcpy(&f_index, buf, sizeof(uint));
    memcpy(&f_size, buf + sizeof(uint), sizeof(uint));
    //printf("%d - File index: %u, size: %u\n", index, f_index, f_size);

    total = f_size;

    do {
      int readsize = total;
      if (readsize > MAX_READSIZE)
	readsize = MAX_READSIZE;

      //int flags = fcntl(sock, F_GETFL, 0);
      //fcntl(sock, F_SETFL, flags & ~O_NONBLOCK);
      n = read(sock, buf, readsize);
            
      total -= n;

    } while (total > 0 && n > 0);

    if (total < 0) {
      printf("failed to read: %d\n", total);
      exit(-1);
    }
    gettimeofday(&stop_time[f_index], NULL);

    file_count++;

    if (period < 0) {
      pthread_mutex_lock(&inc_lock);
      req_file_count--;
      if (req_file_count == 0) {
	req_index++;
	if (req_index < iter) {
	  int *i_index = malloc(sizeof(int));
	  *i_index = req_index;
	  run_iteration(i_index);
	}
      }
      pthread_mutex_unlock(&inc_lock);
    }
  }

  printf("%d - All files received from destination, total: %d\n", index, file_count);
  
  return 0;
}
