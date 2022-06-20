#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <queue>
#include <cstring>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <ctype.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/socket.h>

#include <map>
#include <iterator>
#include <cstring>

#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h> /* hton */

#define perror2(s,e) fprintf(stderr,"%s: %s\n",s,strerror(e))

using namespace std;
//sig func
void sigchld_handler (int sig){ while (waitpid(-1, NULL, WNOHANG) > 0); }
//thread functions
void* communication(void* argp);
void* workers(void* argp);
//main actions in server
void child_server(int newsock,int queue_size,int thread_pool_size,int block_size);


struct arguments{
    int newsock;
    char buf[50];
    int queue_size;
};

struct warg{
    int newsock;
    char* filename;
    int block_size;
};
