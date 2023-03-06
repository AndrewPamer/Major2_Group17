#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>

#define MAXLIST 512

char chistory[20][MAXLIST];
char* HIST_FILE;
int historyIndex;

void myhistory(char* args[]);


void myhistory(char* args[]) {
    if(args[3] != NULL)
    {
        perror("too many arguments\n");
        return;
    }

    if (args[1] == NULL) {
        for (int i = 0; i < historyIndex && chistory[i]; i++) {
            printf("[%i]: %s\n", i + 1, chistory[i]);
        }
    }

    else if(args[2] == NULL)
    {
        if(strcmp(args[1],"-c")==0)
        {
            historyIndex=0;
            for(int i=0;i<20;i++)
            strcpy(chistory[i],"");
            FILE *file = fopen(HIST_FILE,"w");
            fclose(file);

        }
    }

    else if(strcmp(args[1],"-e")==0)
    {
        int i = atoi(args[2]);
        if(i<1 || i>20 || chistory[((historyIndex+i-1)%20)]==NULL)
        {
            printf("index out of reach");
            return;
        }
    }
    else{
        printf("bad arg\n");
    }

}

void addhistory(char *args[]) {
    FILE *file = fopen(HIST_FILE, "w");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }
    char *full_command = calloc(512, sizeof(char));
    char *processed_command = calloc(512, sizeof(char));
    if (args[0] == NULL) {
        return;
    }
    strcat(full_command, args[0]);
    for (int i = 1; args[i]; i++) { // anything past args[1] are parameters
        // add parameters to full parameter string
        strcat(full_command, " ");
        strcat(full_command, args[i]);
    }
    for (int i = 0; full_command[i]; i++) {
        if ((int) full_command[i] < 32 || (int) full_command[i] > 126) {} // if character is not printable, skip it
        else {
            strncat(processed_command, (char*) &full_command[i], 1); // else, add the character to the processed command
        }
    }
    if (strcmp(processed_command, "") == 0) { // if the command is empty
        return; // do nothing
    }
    strcpy(chistory[historyIndex], processed_command); // add the processed command to the history
    for (int i = 0; (i <= historyIndex) && chistory[i]; i++) {
        if (strcmp(chistory[i], "") != 0) { 
            fprintf(file, "%s\n", chistory[i]); // write the history to the file
        }
    }
    historyIndex = (historyIndex + 1) % 20;
    fclose(file);
}

void inithistory() {
    char* home = getenv("HOME");
    char* basename = "/.history.dat";
    HIST_FILE = malloc(strlen(home) + strlen(basename) + 1);
    strcpy(HIST_FILE, home);
    strcat(HIST_FILE, basename);

    FILE *file = fopen(HIST_FILE, "r");
    if (file == NULL) {
        // create file
        file = fopen(HIST_FILE, "w");
        fclose(file);
        file = fopen(HIST_FILE, "r");
    }
    char line[512];
    while(fgets(line, sizeof(line), file) != NULL && historyIndex < 20) { // read the file line by line
        line[strcspn(line, "\n")] = 0; // remove newline
        strcpy(chistory[historyIndex], line);
        historyIndex++;
    }
    printf("historyIndex: %d\n", historyIndex);
    fclose(file);
}
