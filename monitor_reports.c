#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <string.h>


void handle_sigusr1(int signum){

    printf("[Monitor] A new report has been added!\n");   
    fflush(stdout);
}

void handle_sigint(int signum){

    printf("[Monitor] Ending the proccess...\n");

    if(unlink(".monitor_pid") == -1){
        perror("Error while unlink()\n");
    }else{
        printf("[Monitor] The .monitor_pid was deteled.\n");
    }

    exit(0);
}

// sigint - ending signal
// sigusr1 - when a new report has been added
int main(){

    struct sigaction s_int, s_usr1;

    // pointer to a signal_catching function
    s_usr1.sa_handler = handle_sigusr1;
    sigemptyset(&s_usr1.sa_mask); // initialising the mask
    s_usr1.sa_flags = 0;


    return 0;
}