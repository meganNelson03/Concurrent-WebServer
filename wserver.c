// CS 334: Web Server Lab
// Megan Nelson
// April 29, 2019

#include <stdio.h>
#include "request.h"
#include "io_helper.h"
#include <pthread.h>

char default_root[] = ".";
//char default_alg[] = "FIFO";
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t newrequest = PTHREAD_COND_INITIALIZER; 
pthread_cond_t fullBuffer = PTHREAD_COND_INITIALIZER;
pthread_cond_t emptyBuffer = PTHREAD_COND_INITIALIZER;
int fill = 0; 
int count = 0;
int use = 0;
int buffers = 1;
int *buff;


void * threadMethod() {
    pthread_mutex_lock(&lock);
    while (count == 0) {
        pthread_cond_wait(&fullBuffer, &lock); 
    }
    request_handle(buff[use]);
    close_or_die(buff[use]);
    use = (use + 1) % buffers;
    count--;
    pthread_cond_signal(&emptyBuffer);
    pthread_mutex_unlock(&lock);
    
    return NULL;
}

//
// ./wserver [-d <basedir>] [-p <portnum>] [-t <threads>] [-b <buffers>] [-s <schedalg>]
// worker thread are the consumer, buffer is the consumer
int main(int argc, char *argv[]) {

    int c;
    char *root_dir = default_root;
    int port = 10000;
    int threads;
    //char *schedalg = default_alg;

    
    while ((c = getopt(argc, argv, "d:p:t:b:")) != -1)
	switch (c) {
	case 'd':
	    root_dir = optarg;
	    break;
	case 'p':
	    port = atoi(optarg);
	    break;
    case 't':
        threads = atoi(optarg);
        break;
    case 'b':
        buffers = atoi(optarg);
        break;
    //case 's':
      //  schedalg = optarg;
       // break;
	default:
	    fprintf(stderr, "usage: wserver [-d basedir] [-p port] [-t <threads>] [-b <buffers>] [-s <schedalg>]\n");
	    exit(1);
	}

    // run out of this directory
    chdir_or_die(root_dir);

    // create pool of worker threads based on -t arguments,
    // allocate array of buffers to be consumed by worker threads  
    buff = (int *) malloc(buffers * sizeof(int));
    pthread_t *pthreads = (pthread_t *) malloc(threads * sizeof(pthread_t)); 

    // create the number of threads you need
    for (int i=0; i<threads; i++) {
       pthread_create(&pthreads[i], NULL, threadMethod, NULL);       
    }

    // now, get to work
    int listen_fd = open_listen_fd_or_die(port);
    while (1) {
	    struct sockaddr_in client_addr;
	    int client_len = sizeof(client_addr);
	    int conn_fd = 
          accept_or_die(listen_fd, (sockaddr_t *) &client_addr, (socklen_t *) &client_len);
	    // request_handle(conn_fd);
    // MAKE A PUT() METHOD LATER!   
        pthread_mutex_lock(&lock);       
        while (count == buffers) {
            pthread_cond_wait(&emptyBuffer, &lock);
        }   
        buff[fill] = conn_fd;
        fill = (fill + 1) % buffers;
        count++;
        printf("Hello, signalling we are fulll... %d", count);
        pthread_cond_signal(&fullBuffer); 
        pthread_mutex_unlock(&lock);
    // ************************************
	    close_or_die(conn_fd);
    }
    return 0;
}



    


 
