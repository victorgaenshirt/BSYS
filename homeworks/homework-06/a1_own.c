#include <stdio.h> 
#include <unistd.h>
#include <stdlib.h>
#include <time.h>


int main(int argc, char *argv[]) {

    #define BILLION 1E9
    struct timespec starttime, endtime, result;
    double accum, accumfor;
    int iteration = 100;
    
    char* inbuf = "hi\n";
    int fds[2];


    clock_gettime(CLOCK_MONOTONIC_RAW, &starttime);
    for(int i = 0; i < iteration; i++){
        read(fds[0], inbuf, 4);
    }
    clock_gettime(CLOCK_MONOTONIC_RAW, &endtime);

    if((endtime.tv_nsec - starttime.tv_nsec) < 0){//evt usec
        accum = (( endtime.tv_sec - starttime.tv_sec ) - 1 ) * BILLION
        + (( endtime.tv_nsec - starttime.tv_nsec ) + BILLION)
        ;
    }else{
        accum = (endtime.tv_sec - starttime.tv_sec) * BILLION
        + (endtime.tv_nsec - starttime.tv_nsec)
        ;
  }
    printf("systemcall : %lf nanosec\n", accum  /iteration  );
}
