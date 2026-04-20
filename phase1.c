#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>

typedef struct{

    float latitude;
    float longitude;
} GPS_coordinates;

typedef struct{

    int report_id;
    char inspector_name[50];
    GPS_coordinates coord;
    char issues_category[50];
    int severity_level;\
    time_t data;
    char description[100];
} binary_report_file;

void add_function(char *user_role, char *district_id){

    // creating the directory
    mkdir(district_id, 750);

}
// usage example
// ./city_manager --role manager -user alice -- add downtown
int main(int argc, char **argv){

    if(argc < 1){

        exit(1);
    }

    add_function(argv[1], argv[2]);

}