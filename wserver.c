// CS 334: Web Server Lab
// Megan Nelson
// April 29, 2019

#include <stdio.h>
#include "request.h"
#include "io_helper.h"

char default_root[] = ".";
char default_alg[] = "FIFO";

//
// ./wserver [-d <basedir>] [-p <portnum>] [-t <threads>] [-b <buffers>] [-s <schedalg>]
// 
int main(int argc, char *argv[]) {
    int c;
    char *root_dir = default_root;
    int port = 10000;
    int threads;
    int buffers;
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
	    fprintf(stderr, "usage: wserver [-d basedir] [-p port]\n");
	    exit(1);
	}

    // run out of this directory
    chdir_or_die(root_dir);

    // now, get to work
    int listen_fd = open_listen_fd_or_die(port);
    while (1) {
	    struct sockaddr_in client_addr;
	    int client_len = sizeof(client_addr);
	    int conn_fd = 
          accept_or_die(listen_fd, (sockaddr_t *) &client_addr, (socklen_t *) &client_len);
	    request_handle(conn_fd);
	    close_or_die(conn_fd);
    }
    return 0;
}


    


 
