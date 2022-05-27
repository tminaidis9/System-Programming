#include <iostream>
#include <fstream>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <map>
#include <iterator>
#include <cstring>
#include "sniffer2.h"


using namespace std;

extern int errno;

#define SIZE 50
#define PERMS 0666 // set access p e r m i s s i o n s


int main(int argc, char *argv[])
{
    
    // Pipe
    pid_t pd;
    int p[2];
    

    if (pipe(p) == -1) { cout << "pipe error" << endl; };
    // Ο πατέρας ειναι ο Manager, το παιδί είναι ο Listener
    pd=fork();

    if(pd < 0){
        exit(2);
    }

    if(pd == 0){ //Listener: Εκτελεί την inotifywait και γραφει τα αποτελεσματα στο pipe 
        dup2(p[1],1); // close stdout and print everything to p[1]
        execl("/usr/bin/inotifywait", "inotifywait","-m","-q","--format","%f","-e","create","-e","moved_to", "./ex_directory", NULL);
        
        exit(1); // τελος του παιδιου
    }
    
    int i = 0;
    char c;

    while(true) // απερμονο while μεχρι να γινει ^C
    {
        char buffer[SIZE] = {};
        int resize = read(p[0],buffer,SIZE);    
        char fpath[100] =  "np_folder/fifo"; // το path που θα δημιουργουνται τα named_pipes ε ονομα fifo
        c = i + '0'; // καθε καινουργιο φιφο θα εχει και τον αριθμο του
        fpath[14] = c; 
        string right_buff = clean_buffer(buffer); 
        cout << "File is: " << right_buff << endl;
        
        if ( (mkfifo(fpath, PERMS) < 0) && (errno != EEXIST) ) //δημιουργια καινουργιων fifo στο path που επιλεξαμε
            perror("can't create fifo");

        
        pid_t new_pid;
        new_pid = fork(); // Δεύτερο fork οπου: Child == Worker και Parent == Manager

        if(new_pid < 0)
            perror("Second fork FAILED!");
        
        if(new_pid == 0){ // Child
            
            cout << "Opening .." << endl;
            int fd2 = open(fpath,O_RDONLY);
            cout << "Opened.." <<  endl;
            char buffer2[SIZE] = {};

            int resize = read(fd2,buffer2,SIZE); 
            string right_buff2 = clean_buffer(buffer2);
            parser(right_buff2); 
            exit(1);
        }
        else{
            
            cout << "Opening .." << endl;
            int fd3 = open(fpath,O_WRONLY); //για εγγραφη μονο
            cout << "Opened.." <<  endl;

            int len = right_buff.length();
            char rb[len+1];
            strcpy(rb,right_buff.c_str()); 

            if (write(fd3,rb,SIZE) != SIZE) //εγγραφη στο fifo
                perror("write failed\n");

        }
        i++; // num of fifo
        
    }
    

    if(waitpid(pd,NULL,0) <0){
        perror("Waitpid failed");
    }
}

