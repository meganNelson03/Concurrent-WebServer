#include <stdio.h>
#include "request.h"
#include "io_helper.h"
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


char default_root[] = ".";
char default_alg[] = "FIFO";

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t full = PTHREAD_COND_INITIALIZER;

//fill keeps track of next position in buffer
//to put file descriptor in
//volatile int fill = 0;

//use keeps track of next position in buffer
//to take file descriptor from
//volatile int use = 0;

//count keeps track of # of elements in buffer
volatile int count=0;
//int* buff;
volatile int buffers = 1;
volatile int FIFO = 1;

// used to measure file size for SFF scheduler
struct stat st, st1;

struct Node {
    int data;  
    struct Node *next;
};

//struct Node* buffie;


// Print the contents of thread file request list: 
void printList(struct Node *n) {
    printf("The list contains:  "); 
    while (n != NULL) {
        printf("%d, ", n->data);
        n = n->next;
    }
    printf("\n");
}

// When FIFO flag is set to 0, insert new file into queue
// based on file size:
struct Node* insert(struct Node **head, int newData) {
     
    struct Node* newNode = (struct Node*) malloc (sizeof(struct Node));
    struct Node* start = *head;

    newNode->data = newData;

	if (start==NULL){

		printf("Hello, I am an empty list. \n");
		*head = newNode;

	} else {

        if (FIFO==1) {
            
            while (start->next != NULL) {
                start = start->next;
            }
            
            start->next = newNode;            

        } else {
		
		    fstat(newNode->data, &st);
		    fstat(start->data, &st1);
		    off_t size1 = st.st_size;
		    off_t size2 = st1.st_size;
		    //printf("LOOK 1zzz: %lld - size1, %lld - size2\n", (long long) size1, (long long) size2);
		
            // New node is smaller than the head of the list:
            if (size1 < size2){
			    newNode->next = *head;
                *head = newNode;
                printf("After inserting new node at head of the list...");
                printList(buffie);
	        } else {
        
                // Insert the new file in the correct place based on size: 
                while(start != NULL && size1 > size2) {
			    //printf("LOOK HERErepeat: %lld - size1, %lld - size2\n", (long long) st.st_size, (long long) st1.st_size);
                start = start->next;
			    fstat(start->data,&st1);
                printf("After inserting new node (node larger than head)...");
                printList(buffie);
            }        
		
            //printf("LOOK HEREdone: %lld - size1, %lld - size2\n", (long long) st.st_size, (long long) st1.st_size);
            newNode->next = start->next;
            start->next = newNode;
            } 
      }
	} 
	
    return *head;
}

//Called for every new thread that's created:
// Each thread acquires the lock and is put to sleep
// until there is a request to satisfy
void *mythread(void *arg){
	
	while(1) {
		pthread_mutex_lock(&lock);
		//when the buffer (buff) is empty, 
		//wait for signal from main thread
		//while (count==0){
			printf("Going back to sleep\n");
			pthread_cond_wait(&full, &lock);
		//}
		printf("I am alive\n");
		if (FIFO){
			//process handle
			//request_handle(buff[use]);
			//close_or_die(buff[use]);
        
            request_handle(buffie->data);
            close_or_die(buffie->data);
            buffie = buffie->next;

		} else {
            // SFF: 
			request_handle(buffie->data);
			close_or_die(buffie->data);
			buffie = buffie->next;
		}
		

		//update use and count
		//use = (use+1)%buffers;
		count--;

		//signal that one file in buffer has been processed
		pthread_cond_signal(&empty);
		pthread_mutex_unlock(&lock);
	}
	
	return NULL;
}

int main(int argc, char *argv[]) {
    int c;
    char *root_dir = default_root;
    int port = 10000;
	int threads = 1;
	
	char *schedalg = default_alg;
    
    while ((c = getopt(argc, argv, "d:p:t:b:s:")) != -1)
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
	case 's':
	    schedalg = optarg;
	    break;
	default:
	    fprintf(stderr, "usage: wserver [-d basedir] [-p port] [-t threads] [-b buffers] [-s schedalg]\n");
	    exit(1);
	}

    // run out of this directory
    chdir_or_die(root_dir);

	//buffer contains the shared files
	//buff = (int*) malloc(buffers*sizeof(int));
	//buffie = (struct Node*) malloc(/*buffers**/sizeof(struct Node));
	pthread_t *pthreads = (pthread_t*) malloc(threads*sizeof(pthread_t));
	
	//Create pool of worker threads -- the consumers
	for (int i=0;i<threads;i++){
		pthread_create(&pthreads[i], NULL, mythread, NULL);
	}

    int listen_fd = open_listen_fd_or_die(port);
    while (1) {
	    struct sockaddr_in client_addr;
	    int client_len = sizeof(client_addr);
	    int conn_fd = accept_or_die(listen_fd, (sockaddr_t *) &client_addr, (socklen_t *) &client_len);

	    //get the lock and check if the buffer is full 
	    pthread_mutex_lock(&lock);
	    while (count==buffers) {
		    //wait for buffer to be emptied when buffer is full
		    pthread_cond_wait(&empty,&lock);
	    }
	    //printf("add %d", fill);

	    //FIFO
	    if (strcmp(schedalg,"FIFO")==0){

		    //put file descriptor in buffer and update fill, count
		    //buff[fill] = conn_fd;
		    //fill=(fill+1)%buffers;

            buffie = insert(&buffie, conn_fd);
	    } else {
            
            //SFF 
            FIFO=0;
		    buffie=insert(&buffie, conn_fd);
		    //buff[fill] = conn_fd;
		    //buff = bubblesort(buff,buffers);
		    //fill=(fill+1)%buffers;
	    }

	    count++;
	    //signal that file descriptor has been added to buffer
	    pthread_cond_signal(&full);
	    pthread_mutex_unlock(&lock);
    }
    return 0;
}




 
