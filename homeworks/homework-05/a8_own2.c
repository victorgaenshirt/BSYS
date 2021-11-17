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

        int rc2 = fork();
         if(rc2 < 0){
            printf("fork failed");
        } else if(rc2 == 0) {
            //child2
            printf("child2 macht write");
            write(fds[1], message, 4);
                  
        } else {
           //parent
            wait(NULL);
            char inbuf[4]; 
            read(fds[0], inbuf, 4);
            printf("%s\n", inbuf);
        } 
        
        
    } else {
        //parent

    }
    return 0;
}

