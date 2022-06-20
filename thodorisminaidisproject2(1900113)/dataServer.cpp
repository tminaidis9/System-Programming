#include "dataServer.h"

queue<string> q; // η ουρά μέσα στην οποία υπάρχουν όλα τα αρχεία απο τον φακελο που ζητησαμε
string directory;
pthread_t* threads;
int all_files_done;
pthread_mutex_t mtx;

int main(int argc,char* argv[]){
    
    struct sockaddr_in server,client;
    socklen_t clientlen;
    struct sockaddr* serverptr = (struct sockaddr*) &server;
    struct sockaddr* clientptr = (struct sockaddr*) &client;
    struct hostent* rem;
    int sock,port,newsock;
    int bnd,lsn;
    all_files_done = 0;

    int thread_pool_size = atoi(argv[4]);
    int queue_size = atoi(argv[6]);
    int block_size = atoi(argv[8]);
    port = atoi(argv[2]);
    
    // first prints
    printf("Server's parameters are:\n");
    printf("Port: %d\n",port);
    printf("Thread pool size: %d\n",thread_pool_size);
    printf("Block Size: %d\n",block_size);
    printf("Queue Size: %d\n",queue_size);
    printf("Server was successfully initialized...\n");
    //

    threads = (pthread_t*)malloc(thread_pool_size*sizeof(pthread_t));

    if((sock = socket(AF_INET,SOCK_STREAM,0)) < 0)  
        perror2("Problem in server's socket",1);
    
    signal(SIGCHLD, sigchld_handler);

    server.sin_family = AF_INET;;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);

    if(bind(sock,serverptr,sizeof(server)) < 0){
        perror("Problem in bind!");
        exit(2);
    }
    
    int backlog = 5; 
    if(lsn = listen(sock,backlog) < 0){
        perror("Problem in listen");
        exit(3);
    }

    printf("Listening for connections to port %d\n", port);

    while (1) 
    {  
        clientlen = sizeof(client);/* accept connection */
        
        if ((newsock = accept(sock, clientptr, &clientlen))< 0)
        {
            perror("Problem in Accept"); /* Find client's name  */
            exit(1);
        }


        if ((rem = gethostbyaddr((char *) &client.sin_addr.s_addr, sizeof(client.sin_addr.s_addr), client.sin_family))== NULL)
        {
            herror("gethostbyaddr");
            exit(1);
        }

            
        printf("Accepted connection from %s\n", rem->h_name);
        
        switch (fork()) {    /* Create child for serving client */
        case -1:     /* Error */
            perror("fork");
            break;
        case 0:/* Child process */
            close(sock); 
            child_server(newsock,queue_size,thread_pool_size,block_size);
            exit(0);
        }
        
        close(newsock); /* parent closes socket to client */
    }
}   

void child_server(int newsock,int queue_size,int thread_pool_size,int block_size){
    int err;
    char buf[50];
    struct arguments args;
    args.newsock = newsock;
    strcpy(args.buf,buf);
    args.queue_size = queue_size;
    
    //communication thread
    pthread_t comm_thread;
    if(pthread_create(&comm_thread,NULL,communication,(void*)&args) < 0)
        perror2("pthread create",1);


    write(newsock,&block_size,sizeof(int)); //send block_size

    // worker threads creation και ανάθεση file path σε καθενα.
    while(all_files_done == 0 || q.size() != 0){
        for(int i = 0 ; i < queue_size ; i++){
            
            if(all_files_done == 1 && q.size() == 0){
                break;
            }
            while(q.size() == 0){ //wait to give a filename to queue
                continue;

            }
            string fn = q.front();
            char filename[50] = {'\0'}; // αρχικοποίηση του filename χωρίς κάποιο στοιχείο
            for(int i=0;i<fn.size();i++)
                filename[i] = fn[i];
            
            cout << "[Thread: "<< pthread_self() << "]: About to read " << filename << endl;

            struct warg warg;
            warg.filename = filename;
            warg.newsock = newsock;
            warg.block_size = block_size;

            if (err = pthread_create(threads+i,NULL,workers,(void*)&warg))
                perror2("Pthread Create",err);
            
            q.pop();
            
            if(err = pthread_join(*(threads+i),NULL))
                perror2("Pthread Join",err);
        }
    }

    if(pthread_join(comm_thread,NULL))
        perror2("Pthread join last",5);

    printf("Closing connection.\n");
    close(newsock); /* Close socket */
}

void* communication(void* argp){
    char buf[50];
    struct arguments* args = (struct arguments*)argp;
    read(args->newsock, args->buf, 50);


    printf("[Thread: %ld]: About to scan directory %s\n",pthread_self(),args->buf);

    directory = "Server";
    
    int i = 0;
    while((args->buf[i] != ' ') && (args->buf[i] != '\0') && args->buf[i] != '\n'){
        directory.push_back(args->buf[i]);
        i++;
    }

    //at any new file i want an empty Queue
    
    while(!q.empty())
        q.pop();

    for (const auto & entry : filesystem::recursive_directory_iterator(directory)){
        
        if(!entry.is_directory()){
            while(1){
                if(q.size() < args->queue_size){ //push them all in the queue
                    cout <<"[Thread: " << pthread_self() << "]: Adding file: " << entry.path() << endl; //print every file in this folder
                    q.push(entry.path());
                    break;
                }
            }
        }
    }

    all_files_done = 1; 
    return (void*)2;
}

void* workers(void* argp){
    
    if(pthread_mutex_lock(&mtx)){ // lock mutex
        perror2("Pthread Mutex Lock",6);exit(6);}    

    struct warg* args = (struct warg*)argp;
cout << "[Thread: "<< pthread_self() << "]: Received task " << (char*)args->filename << endl;

    char ch;
    //count file size (based on char (bytes))
    int file_size = 0;
    ifstream file;
    file.open(args->filename);
    while(file.get(ch)){
        file_size++;
    }
    // end of counting
    
    file.close();
    write(args->newsock,&file_size,10); // send the size of the file

    write(args->newsock,args->filename,60); // send filename

    file.open(args->filename);
    char block[args->block_size];

    int j= 0; // αφορα τα στοιχεια του καθε μπλοκ
    int counter = 0; // αφορα το συνολο των χαρακτηρων
    while(file.get(ch)){ // read every char of file and create blocks to send
        
        if(j+2 < args->block_size && counter+1 < file_size){
            block[j] = ch;
            j++;
            counter++;
        }
        else{
            block[j] = ch;
            block[j+1] = 0;
            counter++;
            write(args->newsock,block,args->block_size);
            j = 0;
            char block[args->block_size];
        }
        
    }
    file.close();

    
    
    if (pthread_mutex_unlock(&mtx)) { /* Unlock mutex */
        perror2("pthread_mutex_unlock", 7); exit(7); }

    return (void*)2;
}