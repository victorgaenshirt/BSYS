#include <stdio.h> 
#include <unistd.h>
#include <stdlib.h>


int main(int argc, char *argv[]){
    int rc = fork();
    char* message = "Hey";
    int fds[2];
    if (pipe(fds) < 0)
        exit(1);

    if(rc < 0){
        printf("fork failed");
    } else if(rc == 0) {
        //child 1
        
        write(fds[1], message, 4);
        //fds[0] = read
        //fds[1] = write
        printf("pid von child1: %d\n", rc);
    } else {
        //parent
        //wait(NULL);
       // printf("pid von parent nach fork1: %d\n", getpid());
        printf("pid von child mittels rc: %d\n", rc);

        int rc2 = fork();
        if(rc2 < 0){
            printf("fork failed");
        } else if(rc2 == 0) {
            printf("wird ausgeführt");
            //child 2
            char inbuf[4]; 
            read(fds[0], inbuf, 4);
            printf("%s\n", inbuf);
                  
        } else {
           //parent

            
            printf("pid von parent nach fork2: %d\n", getpid());
            printf("rc von child2: %d\n", rc2);
            printf("rc von child1 nach 2. fork: %d\n", rc);
           
        }     
    //parent 1
    }
    return 0;
}

