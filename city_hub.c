#define _POSIX_SOURCE 200809L


#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

// background chil process --> hub_mon

void start_monitor(){

}

void calculate_scores(char **list_of_districts){

    
}

int main(int argc, char **argv){

    if(argc < 2){
        printf("Format: <%s> <function_name>\n");
        exit(1);
    }

    char option[20];
    strcpy(option, argv[1]);

    if(!strcmp(option, "--start_monitor")){
        start_monitor();
    }else if(!strcmp(option, "calculate_scores")){
        // logic for reading the list of districts
    }

    return 0;
}