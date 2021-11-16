#include <stdio.h> 
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]){
    int rc = fork();
    char* message = "THC";
    int pipe1[2];
    int pipe2[2];
    int BILLION = 1000000000;
    struct timespec requestStart, requestEnd;
    int itaration = 100;
    double accum,accumfor;
    if (pipe(pipe1) < 0)
        exit(1);

    clock_gettime(CLOCK_MONOTONIC_RAW, &requestStart);

    if(rc < 0){
        printf("fork failed");
    } else if(rc == 0) {
        //child 1
        for (size_t i = 0; i < itaration; i++) {
        write(pipe1[1], message, sizeof(message)+1); //write an write end of the pipe
        read(pipe2[0], message, sizeof(message)+1); //OS does context switch, because pipe2 is empty
        printf("child: i: %d", i);
        }
        
    } else {
        //parent
        for (size_t i = 0; i < itaration; i++) {
            read(pipe1[0], message, sizeof(message)+1); 
            write(pipe2[1], message, sizeof(message)+1);
            printf("parent: i: %d", i);
        }
    }

    clock_gettime(CLOCK_MONOTONIC_RAW, &requestEnd);


 /*  
    if((requestEnd.tv_nsec - requestStart.tv_nsec) < 0){//evt usec
        accum = (( requestEnd.tv_sec - requestStart.tv_sec ) - 1 ) * BILLION
        + (( requestEnd.tv_nsec - requestStart.tv_nsec ) + BILLION)
        ;
    }else{
        accum = (requestEnd.tv_sec - requestStart.tv_sec) * BILLION
        + (requestEnd.tv_nsec - requestStart.tv_nsec)
        ;
    }

*/

accum = (requestEnd.tv_nsec - requestStart.tv_nsec)  * BILLION;

printf("accum: %d", accum);
return 0;
}

