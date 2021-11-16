#include <stdio.h> 
#include <unistd.h>
#include <stdlib.h>


int main(int argc, char *argv[]){
    int rc = fork();
    if(rc < 0){
        printf("fork failed");
    } else if(rc == 0) {
        close(STDOUT_FILENO);
        printf("asdfjlkdsfjlkdsf\n");
        printf("sdf%s", stderr);
    } else {
        //parent
    

    }

    return 0;
}

