#include "remoteClient.h"

using namespace std;

int main(int argc,char* argv[]){
    
    sockaddr_in server;
    struct sockaddr* serverptr = (struct sockaddr*) &server;
    struct hostent* rem; 
    int cl_sock,sport; //client_socket, server_port
    string server_ip = argv[2]; // take ip of server
    cout << "Server IP: " << server_ip << endl;
    char buffer[50] = {'\0'};

    if((cl_sock = socket(AF_INET,SOCK_STREAM,0)) < 0){ // socket creation
        perror("Problem in client's socket");
        exit(1);
    }


    if ((rem = gethostbyname(server_ip.c_str())) == NULL){ //get host by server_ip
        herror("Problem in gethostbyname");
        exit(2);
    }

    sport = atoi(argv[4]); //Socket Port
    cout << "Port: " << sport << endl;
    server.sin_family = AF_INET;
    memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
    server.sin_port = htons(sport);

    if (connect(cl_sock, serverptr, sizeof(server)) < 0){
        perror("connect problem");
        exit(3);
    }

    printf("Connecting to %s port %d\n", argv[2], sport);

    string dirServer;
    dirServer = dirServer + argv[6];

    cout <<"I want files from " << dirServer << endl;
    
    for(int i = 0 ; i <= dirServer.length(); i++)
        buffer[i] = dirServer[i]; // folder to send from Server to Client
    
    // cout << buffer << endl;
    char buf[60];
    int b[50];
    int* nb = new int;
    char* received_files[100];

    for(int k=0;k<100;k++) // οριο 100 αρχείων που μπορεί να λάβει
        received_files[k] = new char[60]; // keep all the files you receive from this client

    int rec_files = 0; //num of received files
    
    write(cl_sock,buffer,50); // pass the name of the folder

    read(cl_sock,b,sizeof(int));//read  //block_size
    int block_size = *b;
    cout << "Block Size = " << (int)block_size << endl;

    int c = 0;
    int break_while = 1; // συνθηκη για break
    char* new_rec = new char(60);
    while(break_while == 1){

        read(cl_sock,nb,10); //read file size
        int file_size = *nb;
        cout << "File size is " << file_size  << endl;

        read(cl_sock,buf,60); //read filename

        char* filename = new char(60);
        strcmp(filename,buf);

        for(int k=0;k<rec_files;k++){ // check if you have seen again this 
            if(!strcmp(received_files[k],buf)){
                break_while = 0;
                break;
            }
        }

        if(break_while == 0)
            break;

        strcpy(new_rec,buf); 
        received_files[rec_files] = new_rec;
        rec_files++;

        cout << "Received: " << (char*)buf << endl;

        string subfolders = "Client/"; // create the right path on client
        int i = 0;
        char block[block_size]; // τα μπλοκ που καθοριζουν την αποστολη
        
        while(buf[i] != ' ' && buf[i] != '\0'){
            while(buf[i] != '/' && buf[i] != ' ' && buf[i] != '\0'){
                subfolders.push_back(buf[i]);
                i++;
            }
            cout << subfolders << endl;
            if(buf[i] == '/'){ //so, this is a folder
                subfolders.push_back('/');
                mkdir(subfolders.c_str(),S_IRWXU);
            }
            else{ //this is the name file
                char ch;
                read(cl_sock,block,block_size); // first BLOCK
                int j = 0; // counter (per block)
                int counter = 0; // total counter for file size
                ofstream file(subfolders.c_str()); 
                while(file_size > counter){

                    while(file_size > counter && j+1 < block_size){ // write every block
                        file << block[j];
                        j++;
                        counter++;
                    } 
                    if(block_size == j+1 && file_size > counter){ // read block until your file is ready
                        read(cl_sock,block,block_size); 
                        j=0;
                    }

                }
                file.close();
                c++;
            }
            i++;
        }
    }
    

    close(cl_sock);
}