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


    if(sigaction(SIGUSR1, &s_usr1, NULL) == -1){
        perror("Error while sigaction()\n");
        exit(1);
    }

    s_int.sa_handler = handle_sigint;
    sigemptyset(&s_int.sa_mask);
    s_int.sa_flags = 0;

    if(sigaction(SIGINT, &s_int, NULL) == -1){
        perror("Error while sigaction()\n");
        exit(1);
    }

    int fd = open(".monitor_pid", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(fd == -1){
        perror("Error while open()\n");
        exit(1);
    }

    pid_t current_pid = getpid();
    char pid_name[50];
    snprintf(pid_name, sizeof(pid_name), "%d\n", current_pid);

    //writing the pid in the .monitor_pid
    write(fd, pid_name, strlen(pid_name));
    close(fd);

    printf("[Monitor] PID: %d saved in '.monitor_pid'\n", current_pid);
    fflush(stdout);

    while(1){
        sleep(10);
    }

    return 0;
}