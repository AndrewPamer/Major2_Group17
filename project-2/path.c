#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "path.h"

char* PATH_FILE;

void init_path() {
    char* home = getenv("HOME");
    char* basename = "/.path.dat";
    PATH_FILE = malloc(strlen(home) + strlen(basename) + 1);
    strcpy(PATH_FILE, home);
    strcat(PATH_FILE, basename);
    FILE *path_file = fopen(PATH_FILE, "r");
    if (path_file == NULL) { // if the file doesn't exist
        path_file = fopen(PATH_FILE, "w");  // create the file
        fprintf(path_file, "%s", getenv("PATH"));
        fclose(path_file);
        path_file = fopen(PATH_FILE, "r");  // reopen the file
    }
    char *buffer = (char*) calloc(513, sizeof(char)); // 513 to allow for trailing null character
    fgets(buffer, 513, path_file); // path is always the first line
    buffer[strcspn(buffer, "\n")] = 0; // remove newline
    
    // strcpy(new_path, buffer);
    // strcat(new_path, ":");
    // strcat(new_path, old_path);
    setenv("PATH", buffer, 1); // set the path
}

void path(char* args[]) {
    char* path = getenv("PATH"); // get the path
    if (args[1] == NULL) { // if no arguments
        printf("PATH: %s\n", path); // print the path
    } else if (strcmp(args[1], "+") == 0) { // if the first argument is +
        char buffer[513];
        strcpy(buffer, path); // copy the path
        strcat(buffer, ":"); // add a colon
        strcat(buffer, args[2]); // add the new path
        setenv("PATH", buffer, 1); // set the new path
    } else if (strcmp(args[1], "-") == 0) { // if the first argument is -
        // remove entry from path
        char *buffer = (char*) calloc(513, sizeof(char)); // 513 to allow for trailing null character
        char *new_path = (char*) calloc(513, sizeof(char)); // 513 to allow for trailing null character
        strcpy(buffer, path); // copy the path
        char* token = strtok(path, ":"); // tokenize the path
        while (token != NULL) { 
            if (strcmp(token, args[2]) == 0) { // if the token is the entry to remove
                // remove token
                token = strtok(NULL, ":");
            } else {
                strcat(new_path, token); // add the token to the new path
                strcat(new_path, ":"); // add a colon
                token = strtok(NULL, ":"); // get the next token

            }
        }
        setenv("PATH", new_path, 1); // set the new path
    } else {
        printf("path: invalid option -- %s\n", args[1]); // print invalid option
    }
    write_path(); // write the path to the file
}

void write_path() {
    char* path = getenv("PATH"); // get the path
    FILE* file = fopen(PATH_FILE, "w"); // open the file
    fprintf(file, "%s", path); // write the path
    fclose(file); // close the file
}