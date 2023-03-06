//Andrew Pamer
//ajp0317
//cd.c
//Implements change directory command(cd).

#include <stdlib.h> //for getenv()
#include <unistd.h> //for chdir()
#include <string.h> //for NULL
#include <stdio.h>
int cd(char* path)
{

    if(!path || *path ==0) {
        char *home = getenv("HOME");    //get the Home directory
        if(chdir(home) != 0)    //error checking
            perror("Error changing to home directory!");
    }

    //Use 'cd ..' to go back a directory. Use 'cd ' or 'cd /' to go to the home directory
    else {
        if(chdir(path) != 0)    //Error checking
            perror("Error changing directory!");
    }

    return 1;
}
