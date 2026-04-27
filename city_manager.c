#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

typedef struct{

    float latitude;
    float longitude;
} GPS_coordinates;

typedef struct{

    int report_id;
    char user_name[50];
    GPS_coordinates coord;
    char issues_category[50];
    int severity_level; // 1 - minor; 2 - moderate; 3 - critical
    time_t timestamp;
    char description[200];
} binary_report_file;

void add_function(char *user_role, char *user_name, char *district_id){

    // creating the district directory
    // 0750 -> rwxr-x---
    if(mkdir(district_id, 0750) == -1){

        if(errno == EEXIST){
            printf("The directory already exists!\n");
        } else {
            perror("Error while mkdir()\n");
            exit(1);
        }
    } else {

        if(chmod(district_id, 0750) == -1){
            perror("Error while chmod() for reports.dat\n");
            exit(1);
        }
    }

    // creating the new report
    binary_report_file new_file;

    strcpy(new_file.user_name, user_name);

    new_file.report_id = 0;
    new_file.timestamp = time(NULL);

    printf("Enter coordintes...\n");
    printf("Latitude: ");
    scanf("%f", &new_file.coord.latitude);
    printf("Latitude: ");
    scanf("%f", &new_file.coord.longitude);

    printf("Enter issue category: ");
    scanf("%49s", new_file.issues_category);

    printf("Choose severity level...\n");
    printf("1 - minor | 2 - moderate | 3 - critical\n");
    printf("Enter severity level: ");
    scanf("%d", &new_file.severity_level);

    int c;
    while ((c = getchar()) != '\n');

    printf("Write a brief description of the event... (maximum 200 characters)\n");
    fgets(new_file.description, sizeof(new_file.description), stdin);

    printf("%d\n %s\n %f %f\n %s\n %d\n %ld\n %s", new_file.report_id, new_file.user_name, new_file.coord.latitude, 
        new_file.coord.longitude, new_file.issues_category, new_file.severity_level, new_file.timestamp, new_file.description);


    char filepath[100];
    snprintf(filepath, sizeof(filepath), "%s/reports.dat", district_id);
    
    // 0664 -> rw-rw-r--
    int fd = open(filepath, O_RDWR | O_CREAT | O_APPEND, 0664);
    
    if(fd == -1){
        perror("Error while open()\n");
        exit(1);
    }

    if(chmod(filepath, 0664) == -1){
        perror("Error while chmod()\n");
        exit(1);
    }

    off_t filesize = lseek(fd, 0, SEEK_END);

    if(filesize == 0){
        new_file.report_id = 0;
    } else {

        binary_report_file last_report;

        lseek(fd, -sizeof(binary_report_file), SEEK_CUR);

        if(read(fd, &last_report, sizeof(binary_report_file)) > 0){
            new_file.report_id = last_report.report_id + 1;
        } else {
            perror("Error while read()\n");
            new_file.report_id = 0;
        }
    }

    ssize_t bytes_wr = write(fd, &new_file, sizeof(binary_report_file));

    if(bytes_wr == -1){
        perror("Error while write()\n");
        exit(1);
    } else if(bytes_wr != sizeof(binary_report_file)){
        perror("The report's structure was not written properly!\n");
    } else {
        printf("The report was saved successfully\n");
    }

    // district.cfg  0640 rw-rw----
    char district_path[100];
    snprintf(district_path, sizeof(district_path), "%s/district.cfg", district_id);

    int fd_cfg = open(district_path, O_WRONLY | O_CREAT | O_EXCL, 0640);
    if(fd_cfg != -1){
        if(chmod(district_path, 0640) == -1){
            perror("Error while chmod() for district.cfg\n");
            exit(1);
        }

        char cfg_content[] = "Minimum severity threshold = 2\n";
        
        write(fd_cfg, cfg_content, sizeof(cfg_content));
        close(fd_cfg);
        printf("The district.cfg file was created successfully\n");
    }

    // logged_district 
    char logged_path[100];
    snprintf(logged_path, sizeof(logged_path), "%s/logged_district", district_id);

    int fd_logged = open(logged_path, O_CREAT, O_APPEND, 0664);
    if(fd_logged != -1){
        chmod(logged_path, 0644);
        close(fd_logged);
    }

    struct stat statbuf;
    if(stat(logged_path, &statbuf) == -1){
        perror("Error while stat()\n");
        exit(1);
    } else {

        int permission = 0;

        if(strcmp(user_role, "manager") == 0){
            if(statbuf.st_mode & S_IWUSR){
                permission = 1;
            } else {
                printf("Restriction detected, the user cannot do this action\n");
            }
        }

        if(permission){

            int fd_logged = open(logged_path, O_WRONLY | O_APPEND);
            if(fd_logged != -1){
                
                char logged_entry[200];

                snprintf(logged_entry, sizeof(logged_entry), "%ld %s %s action performed: add\n", new_file.timestamp, user_role, user_name);

                write(fd_logged, logged_entry, strlen(logged_entry));
                close(fd_logged);
            } else {
                perror("Error while open()\n");
            }
        }
    }

    char symlink_name[100];

    snprintf(symlink_name, sizeof(symlink_name), "active_reports-%s", district_id);

    if(symlink(filepath, symlink_name) == -1){
        if(errno != EEXIST){
            perror("Error creating symlink!");
        }
    } else {
        printf("Symlink created successfully!\n");
    }

    close(fd);
}

void mode_to_string_converter(mode_t mode, char *permissions){

    strcpy(permissions, "---------");

    if(mode & S_IRUSR)
        permissions[0] = 'r';
    if(mode & S_IWUSR)
        permissions[1] = 'w';
    if(mode & S_IXUSR)
        permissions[2] = 'x';

    if(mode & S_IRGRP)
        permissions[3] = 'r';
    if(mode & S_IWGRP)
        permissions[4] = 'w';
    if(mode & S_IXGRP)
        permissions[5] = 'x';

    if(mode & S_IROTH)
        permissions[6] = 'r';
    if(mode & S_IWOTH)
        permissions[7] = 'w';
    if(mode & S_IXOTH)
        permissions[8] = 'x';
}

void list_function(char *district_name){

    char filepath[100];
    snprintf(filepath, sizeof(filepath), "%s/reports.dat", district_name);

    struct stat statbuf;

    if(stat(filepath, &statbuf) == -1){
        perror("Error while stat()\n");
        return; // the file probably doesn't exist
    }

    char permissions[10];
    mode_to_string_converter(statbuf.st_mode, permissions);

    char *time_print = ctime(&statbuf.st_mtime);
    time_print[strcspn(time_print, "\n")] = 0;

    printf("%s %ld %s\n", permissions, statbuf.st_size, time_print);

    int fd = open(filepath, O_RDONLY); 
    if(fd == -1){
        perror("Error while open()\n");
        return;
    }

    binary_report_file report;
    int index = 0;

    while(read(fd, &report, sizeof(binary_report_file)) > 0){
        printf("%-2d %-18s %-14s %d\n", report.report_id, report.user_name, report.issues_category, report.severity_level);
        index++;
    }

    if(!index)
        printf("There are no report registered in this discrict!\n");

    close(fd);
}

void view_function(char *district_name, int report_id){

    char symlink_path[100];
    snprintf(symlink_path, sizeof(symlink_path), "active_reports-%s", district_name);

    int fd = open(symlink_path, O_RDONLY);
    if(fd == -1){
        perror("Error while open()\n");
        return;
    }

    // getting the offset for the required report using the id
    off_t report_offset = report_id * sizeof(binary_report_file);
    // finding the exact report
    off_t cursor = lseek(fd, report_offset, SEEK_SET);

    if(cursor == -1){
        perror("Error finding the required report!\n");
        close(fd);
        return;
    }

    binary_report_file required_report;
    
    ssize_t bytes_read = read(fd, &required_report, sizeof(binary_report_file));

    if(bytes_read == sizeof(binary_report_file)){

        if(required_report.report_id == report_id){

            char *time_print = ctime(&required_report.timestamp);
            time_print[strcspn(time_print, "\n")] = 0;

            printf("ID: %d\n", required_report.report_id);
            printf("User name: %s\n", required_report.user_name);
            printf("Coordinates: %f %f\n", required_report.coord.latitude, required_report.coord.longitude);
            printf("Issue category: %s\n", required_report.issues_category);
            printf("Severity level: %d\n", required_report.severity_level);
            printf("Timestamp: %s\n", time_print);
            printf("Description: %s\n", required_report.description);
        }
    } else if(bytes_read == 0){
        printf("No report found with the id %d\n", report_id);
    } else {
        printf("Error while reading the report!\n");
    }

    close(fd);
}

void remove_function(char *district_id, char *user_role, int report_id){

    if(strcmp(user_role, "manager")){
        printf("Permission denied! Only the manager cand perform this action.\n");
        return;
    }

    char symlink_path[100];
    snprintf(symlink_path, sizeof(symlink_path), "active_reports-%s", district_id);

    int fd = open(symlink_path, O_RDWR);
    if(fd == -1){
        perror("Error while open()\n");
        return;
    }

    off_t initial_size = lseek(fd, 0, SEEK_END);
    off_t removed_report_offset = sizeof(binary_report_file) * report_id; 

    if(removed_report_offset >= initial_size){
        printf("The report is not valid\n");
        close(fd);
        return;
    }

    off_t read_cursor = sizeof(binary_report_file) + removed_report_offset;
    off_t write_cursor = removed_report_offset;

    binary_report_file tmp;

    // overwriting the target report
    while(read_cursor < initial_size){
        lseek(fd, read_cursor, SEEK_SET);
        read(fd, &tmp, sizeof(binary_report_file));

        lseek(fd, write_cursor, SEEK_SET);
        write(fd, &tmp, sizeof(binary_report_file));

        read_cursor += sizeof(binary_report_file);
        write_cursor += sizeof(binary_report_file);
    }

    off_t new_size = initial_size - sizeof(binary_report_file);
    if(ftruncate(fd, new_size) == -1){
        perror("Error while ftruncate()\n");
    } else {
        printf("The report with the id %d was deteled successfully!\n", report_id);
    }
    
    close(fd);

}

void update_threshold(char *district_id, char *user_role, char *user_name, int new_threshold){

    if(strcmp(user_role, "manager")){
        printf("Permission denied! Only the manager cand perform this action.\n");
        return;
    }

    char filepath[100];
    snprintf(filepath, sizeof(filepath), "%s/district.cfg", district_id);

    struct stat statbuf;
    if(stat(filepath, &statbuf) == -1){
        perror("Error while stat()\n");
        return;
    }

    if((statbuf.st_mode & 0777) != 0640){
        printf("Diagnostic: The permissions for the district.cfg file have been altered\n");
        return;
    }

    int fd = open(filepath, O_WRONLY | O_TRUNC);
    if(fd == -1){
        perror("While while open()\n");
        return;
    }

    char new_content[100];
    snprintf(new_content, sizeof(new_content), "Minimum severity threshold = %d\n", new_threshold);

    if(write(fd, new_content, strlen(new_content)) == -1){
        perror("Error while write()\n");
    } 

    close(fd);

    char logged_path[100];
    snprintf(logged_path, sizeof(logged_path), "%s/logged_district", district_id);

    int fd_logged = open(logged_path, O_WRONLY | O_APPEND);
    if(fd_logged != -1){
        char logged_entry[200];
        snprintf(logged_entry, sizeof(logged_entry), "%ld %s %s action performed: update_threshold\n", time(NULL), user_role, user_name);
        write(fd_logged, logged_entry, strlen(logged_entry));
        close(fd_logged);
    } else {
        perror("Warning: Could not open logged_district to record the action\n");
    }

}

int parse_condition(const char *input, char *field, char *op, char *value) {

    if (sscanf(input, "%[^:]:%[^:]:%s", field, op, value) == 3) {
        return 1; 
    }
    return 0; 
}

int match_condition(binary_report_file *r, const char *field, const char *op, const char *value) {
    
    // Severity field
    if (strcmp(field, "severity") == 0) {
        int val = atoi(value); 
        
        if (strcmp(op, "==") == 0) return r->severity_level == val;
        if (strcmp(op, "!=") == 0) return r->severity_level != val;
        if (strcmp(op, "<") == 0)  return r->severity_level < val;
        if (strcmp(op, "<=") == 0) return r->severity_level <= val;
        if (strcmp(op, ">") == 0)  return r->severity_level > val;
        if (strcmp(op, ">=") == 0) return r->severity_level >= val;
    }
    // Timestamp field
    else if (strcmp(field, "timestamp") == 0) {
        long val = atol(value); 
        
        if (strcmp(op, "==") == 0) return r->timestamp == val;
        if (strcmp(op, "!=") == 0) return r->timestamp != val;
        if (strcmp(op, "<") == 0)  return r->timestamp < val;
        if (strcmp(op, "<=") == 0) return r->timestamp <= val;
        if (strcmp(op, ">") == 0)  return r->timestamp > val;
        if (strcmp(op, ">=") == 0) return r->timestamp >= val;
    }
    // Category field
    else if (strcmp(field, "category") == 0) {

        if (strcmp(op, "==") == 0) return strcmp(r->issues_category, value) == 0;
        if (strcmp(op, "!=") == 0) return strcmp(r->issues_category, value) != 0;
    }
    // Inspector field
    else if (strcmp(field, "inspector") == 0) {
        if (strcmp(op, "==") == 0) return strcmp(r->user_name, value) == 0;
        if (strcmp(op, "!=") == 0) return strcmp(r->user_name, value) != 0;
    }
    
    return 0; 
}

void filter_function(char *district_id, int condition_count, char **conditions) {

    char symlink_path[100];
    snprintf(symlink_path, sizeof(symlink_path), "active_reports-%s", district_id);

    int fd = open(symlink_path, O_RDONLY);
    if (fd == -1) {
        perror("Error while open()\n");
        return;
    }

    char fields[10][50];
    char ops[10][5];
    char values[10][50];

    for (int i = 0; i < condition_count; i++) {
        if (!parse_condition(conditions[i], fields[i], ops[i], values[i])) {
            printf("Error: Invalid format for '%s'. Format: field:op:value\n", conditions[i]);
            close(fd);
            return;
        }
    }

    binary_report_file report;
    int found_any = 0;

    while (read(fd, &report, sizeof(binary_report_file)) > 0) {
        int matches_all = 1; 

        for (int i = 0; i < condition_count; i++) {
            if (match_condition(&report, fields[i], ops[i], values[i]) == 0) {
                matches_all = 0; 
                break;
            }
        }

        if (matches_all) {
            printf("%-2d %-18s %-14s %d\n",
                   report.report_id,
                   report.user_name,
                   report.issues_category,
                   report.severity_level);
            found_any = 1;
        }
    }

    if (!found_any) {
        printf("No report found...\n");
    }

    close(fd);
}

void verify_symlinks_integrity(){

    DIR *directory = opendir(".");
    if(!directory){
        perror("Error while opendir()\n");
        return; 
    }

    struct dirent *dir_entry;
    struct stat lstatbuf, statbuf;
    // reading the directory contents
    while((dir_entry = readdir(directory))){

        // Looking for the right files
        if(!strncmp(dir_entry->d_name, "active_reports-", 15)){
            if(lstat(dir_entry->d_name, &lstatbuf) != -1){
                // Verify if symbolic link
                if(S_ISLNK(lstatbuf.st_mode)){
                    if(stat(dir_entry->d_name, &statbuf) == -1){
                        // if no connection found
                        if(errno == ENOENT){
                            // deleting the dangling link
                            if(unlink(dir_entry->d_name) == -1){
                                perror("Error while unlink()\n");
                            }
                        }
                    }
                }
            }
        }
    }

    closedir(directory);
}

// usage example
// ./city_manager --role manager --user alice --add downtown
int main(int argc, char **argv){

    if(argc < 1){

        printf("Format: <%s> --role <user_role> --user <user_name> --<function_name> <details>\n", argv[0]);
        exit(1);
    }

    char user_role[50];
    char user_name[50];
    char district_id[50];
    char option[50];

    strcpy(user_role, argv[2]);
    strcpy(user_name, argv[4]);
    strcpy(option, argv[5]);
    strcpy(district_id, argv[6]);

    verify_symlinks_integrity();

    if(!strcmp(option, "--view")){
        int report_id = atoi(argv[7]);
        view_function(district_id, report_id);
    } else if(!strcmp(option, "--remove")){
        int report_id = atoi(argv[7]);
        remove_function(district_id, user_role, report_id);
    } else if(!strcmp(option, "--list")){
        list_function(district_id);
    } else if(!strcmp(option, "--add")){
        add_function(user_role, user_name, district_id);
    } else if(!strcmp(option, "--update_threshold")){
        int new_threshold = atoi(argv[7]);
        update_threshold(district_id, user_role, user_name, new_threshold);
    }else if(!strcmp(option, "--filter")){
        int condition_count = argc - 7;
        filter_function(district_id, condition_count, &argv[7]);
    }else{
        printf("Format: <%s> --role <user_role> --user <user_name> --<function_name> <district_id> <details>\n", argv[0]);
        exit(1);
    }
}