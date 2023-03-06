#include <stdio.h>      //for input/output
#include <stdlib.h>     //for exit();
#include <string.h>     //for string manipulation
#include <stdbool.h>    //for bool
#include <unistd.h>     //for parent/child process and functions like chdir(), getpid(), pipe(), ect...
#include <pthread.h>    //for using mutex locks (may not be needed)
#include <sys/types.h>  //for creating threads
#include <sys/wait.h>   //for using wait() and waitpid()
#include <fcntl.h>

#include "major2.h"     //Header file
#include "alias.h"
#include "history.h"
#include "path.h"

#define MAXCHARS 512    //512 is the maxium value for input

char inpLine[MAXCHARS]; //array to hold entire user input
char bbuffer[MAXCHARS];
char *userCmds[100];    //array to hold commands user inputs
char *bCmds[100] = {0};
// array to hold parameters for each command
// withParams[i][0] will always be the base command, anything after that will be parameters given
char *withParams[100][512]; 
char *pipeCmds[100][512];
char* prompt;

int main(int argc, char *argv[])  {
    //TODO: read path as required
    prompt = (char*) calloc(512, sizeof(char)); // allocate memory for the prompt
    FILE *prompt_file = fopen("prompt.dat", "r"); // open the prompt file
    if (prompt_file == NULL) { // if the file does not exist
        prompt_file = fopen("prompt.dat", "w"); // create the file
        fprintf(prompt_file, ">>>"); // write the default prompt
        fclose(prompt_file); // close the file
        prompt_file = fopen("prompt.dat", "r"); // open the file
    }
    fgets(prompt, 513, prompt_file); // read the file into the prompt
    strcat(prompt, " "); // add a space to the end of the prompt
    inithistory(); // initialize the history
    init_path(); // initialize the path
    initialize_alias(); //initialize the alias list
    if(argc > 1) {
        FILE *bFile;
        bFile = fopen(argv[1], "r");
        if(!bFile) {
            perror("Cannot Open File!");
            exit(-1);
        }
        else {
		int bIndex = 0;
		while(1) {
			bCmds[bIndex] = malloc(512);
			fgets(bCmds[bIndex], MAXCHARS, bFile);
			if(feof(bFile))
				break;
			bIndex++;
		}
		int inpIndex = 0;
		for(int i = 0; bCmds[i]; i++) {
			for(int j = 0; *(*(bCmds+i)+j); j++) {
				if(*(*(bCmds+i)+j) == '\n')
					inpLine[inpIndex] = ';';
				inpLine[inpIndex] = *(*(bCmds + i) + j);
				inpIndex++;
			}
		}
        }
        fclose(bFile);
        batchMode();
    }
    else {
        interactiveMode();
    }

    /*
    There needs to be two modes: interactive and bash, so we need to check what the mode should be based on the user input.
    Bash mode will require a file and at least one command.
    Interactive mode will only require at least one command.
    Bash mode should NOT display a prompt. It should just echo each line read from the batch file before executing.

    We can assume arguments are separated by whitespace.
    We do not have to worry about special characters except the redirection and pipeline operator.

    If there are multiple commands in a single line, they will be separated by a semicolon(;).
    Mutiple commands should be done sequentially (Each command should run regardless if the previous one failed).
    Use wait() or waitpid() to do this.

    We can assume that the maximum command line will be 512 bytes (including \n).
    But, there should NOT be a limit on the maxium number of commands in a single line.

    Use stderr for error checking.
    */

    return 0;
}

//In interactive mode the user will input commands from the keyboard.
void interactiveMode()  {

    memset(inpLine, 0, sizeof inpLine); // clear the input line
    memset(userCmds, 0, sizeof userCmds); // clear the user commands
    memset(withParams, 0, sizeof withParams); // clear the user commands

    printf("%s", prompt);
    fgets(inpLine, MAXCHARS, stdin);    //get user input
    split();
    execute(0);
    interactiveMode();
}

/****************************************************************/
//In batch mode, commands are executed from an input file.
void batchMode() {
    split();
    execute(1);
    interactiveMode();
}

/****************************************************************/

//Split the input into separate individual commands.
void split() {
    inpLine[strlen(inpLine)-1] = '\0';  //user input ends with \n. This may cause issues when comparing strings. Replace it with \0 (Null termination Character).
    char *cmd = strtok(inpLine, ";");   //used for splitting the input string into multiple strings for each command.
    int i = 0;
    while (cmd) {
	
	if(strcmp(cmd, " ") !=0) {	//If the command is nothing( ; ; ;) do not add it
        	userCmds[i]=cmd;
		i++;
	}
        cmd=strtok(NULL, ";\n");
    }

    for (int i = 0; userCmds[i]; i++) {  // iterate over given commands
        char *cmd = strtok(userCmds[i], " ");  // split command and parameters
        int j = 0;
        while (cmd) {  // iterate over parameters
            withParams[i][j]=cmd;  // put parameters into withParams[i][j];
            if(strcmp(cmd, "cd") != 0)  //Directory names can have spaces. We don't want to split if the command is cd.
                cmd=strtok(NULL, " ");  // split next parameter
            else
                cmd=strtok(NULL,"");    //if the command is cd, don't split. Keep the directory parameter intact
            j++;
        }
    }

    //Remove any extra spaces from the commands.
    int q = 0;
    for(int i = 0; userCmds[i]; i++) {
        for(int j = 0; *(*(userCmds + i) + j); j++) {
            if(*(*(userCmds + i ) + j) != ' ') {
                *(*(userCmds + i) + q) = *(*(userCmds + i) + j);
                q++;
            }
        }
        *(*(userCmds + i) + q) = '\0';
        q=0;
    }
}

/****************************************************************/
//Execute commands based on user input.
void execute(bool isBatch) {
	int fdout = dup(1);
	int fdin = dup(0);
    int k = 0;
    int bPrintIndex=0;
    bool exit_var = false;
    while(userCmds[k]) {
        // remove prepending spaces
        if(isBatch){
		printf("%u\n", strcspn(bCmds[bPrintIndex], userCmds[k]));
		if(strcspn(bCmds[bPrintIndex], userCmds[k]) == 0) {
			printf("~~~EXECUTING: %s\n", bCmds[bPrintIndex]);
			bPrintIndex++;
		}
	}     //Print commands if the input mode is Batch mode.
        AliasNode* node = getAlias(userCmds[k]);
        if (node != NULL) { //if the command is an alias
            char params[512]; //create a new array to hold the parameters
            strcpy(params, node->parameters); //copy the alias parameters into the new array
            strcat(params, " "); //add a space to the end of the parameters
            // prepends the parameters to the command
            for (int i = 1; withParams[k][i]; i++) { //iterate over the parameters
                strcat(params, withParams[k][i]); //add the parameters to the end of the alias parameters
                strcat(params, " "); //add a space to the end of the parameters
            }

            strcpy(withParams[k][0], params); //set the command to the alias command
            char* param = strtok(params, " "); //split the parameters
            for (int i = 1; param; i++) { //iterate over the parameters
                withParams[k][i] = param; //set the parameters to the array
                param = strtok(NULL, " "); //split the next parameter
            }
            strcpy(userCmds[k], node->command); //set the command to the alias command
        }
        addhistory(withParams[k]);

        int pipeNum = 0;
        for(int i = 0; withParams[k][i]; i++) {
            if(strcmp(withParams[k][i], "|")==0) {
                pipeNum++;
                for(int j = i + 1; withParams[k][j]; j++) {
                    if(strcmp(withParams[k][j], "|")==0) {
                        pipeNum++;
                    }
                }
                if(pipeNum > 2) {
                    perror("Too many pipes\n");	//There can only be 2 pipes in one command
                    exit(-1);
                }
                int x = 0;	//x = first array index (command)
                int y = 0;	//y = seconds array index (params)
                //Set up the commands
                for(int m = 0; withParams[k][m]; m++) {
                    if(strcmp(withParams[k][m], "|") == 0) {
                        x++;
                        y=0;
                    }
                    else {
                        if(x==0 && y==0) 
                            pipeCmds[x][y] = userCmds[k];
                        else
                            pipeCmds[x][y] = withParams[k][m];
                        y++;
                    }
                }
                int pipefd[2];	//Create pipe
                int pipefdOpt[2];   //Create second pipe for third command
                pipe(pipefd);
                pipe(pipefdOpt);
                //1 is write end
                //0 is read end
                x=0;
                if(fork() == 0) {
                    dup2(pipefd[1], fileno(stdout));    //Set output of pipe to standard output
                    close(pipefd[0]);   //close pipe ends
                    close(pipefd[1]);
                    execvp(pipeCmds[0][0], pipeCmds[0]);    //execute commands
                    exit(1);
                }
                if(fork() == 0) {
                    dup2(pipefd[0], fileno(stdin)); //set input of pipe to standard input
                    close(pipefd[0]);   //close pipe ends
                    close(pipefd[1]);
                    if(pipeCmds[2][0]) {    //if there is another command
                        dup2(pipefdOpt[1], fileno(stdout)); //set the output of pipe to standard output
                        close(pipefdOpt[0]);    //close pipe ends
                        close(pipefdOpt[1]);
                    }
                    execvp(pipeCmds[1][0], pipeCmds[1]);    //execute commands
                    exit(2);
                }
                if(pipeCmds[2][0])  //if there is a third command
                    if(fork()==0) {
                        dup2(pipefdOpt[0], fileno(stdin));  //set input of pipe to standard input
                        close(pipefd[0]);   //close pipe ends
                        close(pipefd[1]);
                        close(pipefdOpt[0]);    //close pipe ends
                        close(pipefdOpt[1]);
                        execvp(pipeCmds[2][0], pipeCmds[2]);    //execute
                        exit(3);
                    }
                close(pipefd[0]);   //close pipe ends
                close(pipefd[1]);
                close(pipefdOpt[0]);    //close other pipe ends
                close(pipefdOpt[1]);
                wait(0);    //wait for child processes to finish
                wait(0);
                wait(0);
                memset(pipeCmds, 0, sizeof pipeCmds);   //set pipeCmds to null
                dup2(fdout, fileno(stdout));    //reset standard output
                dup2(fdin, fileno(stdin));  //reset standard input
                if(withParams[k+1][0])  //if there is another command 
                    k++;    //go to that one
                else {  //else we are done
                    memset(inpLine, 0, sizeof inpLine);
                    memset(userCmds, 0, sizeof userCmds);
                    memset(withParams, 0, sizeof withParams);
                    interactiveMode();
                }	
            }
        }
        for(int i = 0; withParams[k][i]; i++) {
        if(strcmp(withParams[k][i], ">")==0) {	//if there is a '>' symbol
            int fd = open(withParams[k][i+1], O_WRONLY | O_CREAT, 0666);	//open file with write only parameters
            if(fd == -1) {
                perror("Error opening output file!");	//error checking 
                break;
            }
            printf("%s\n", userCmds[k]);	//testing
            printf("%s\n", withParams[k][i]);	//testing
            withParams[k][i] = NULL;	//remove the parameter (the '>')
            withParams[k][i+1] = NULL;	//remove the next parameter (the file name)
            dup2(fd, fileno(stdout));	//set stdout to the file descriptor
            close(fd);
        }
        }
        for(int i = 0; withParams[k][i]; i++) {
            if(strcmp(withParams[k][i], "<") ==0) {	//if there is a '<' symbol
                int fd = open(withParams[k][i+1], O_RDONLY);	//open read only file
                if(fd==-1) {
                    perror("Error opening input file!");	//error checking
                    break;
                }
                dup2(fd, fileno(stdin));	//set standard input to open file
                withParams[k][i+1] = NULL;	//set parameter to null
                printf("\n%s\n", withParams[k][i]);	//test
                scanf("%s", withParams[k][i]);
                printf("%s\n", withParams[k][i]);	//tes
                close(fd);
            }
        }
	//printf("%s", withParams[k][2]);
        if(strcmp(userCmds[k], "cd")==0) {
            //call cd function
            cd(withParams[k][1]);
        }
        else if (strcmp(userCmds[k], "alias") == 0) {
            alias(withParams[k]);
        }
        else if(strcmp(userCmds[k], "exit")==0) {
            exit_var = true;
        }
        else if(strcmp(userCmds[k], "myhistory")==0) {
            strcpy(withParams[k][0], userCmds[k]);
            myhistory(withParams[k]);
        }
        else if (strcmp(userCmds[k], "path") == 0) {
            path(withParams[k]);
        }
        else if (strcmp(userCmds[k], "prompt") == 0) {
            prompt_command(withParams[k]);
        }

        //and so on...
        else {
            //call fork function
            pid_t pid = fork();
            int status;
            setpgid(pid, pid);
            signal(SIGTTOU, SIG_IGN);   //ignore SIGTTOU
            tcsetpgrp(0, getpgrp());    //set foreground process to child process
            if (pid == 0) {

                execvp(userCmds[k], withParams[k]);
                // if process continues, execvp failed
                printf("%s: command not found\n", userCmds[k]);
                exit(0);
            }
            else if (pid < 0) {
                printf("Fork failed\n");
                exit(0);
            }
            else {
                waitpid(pid, &status, 0);
                tcsetpgrp(0, getpgrp()); //set foreground process back to parent process
            }
        }
        k++;
	dup2(fdout, fileno(stdout));	//reset stdout
	dup2(fdin, fileno(stdin));	//reset stdin
	close(fdout);
	close(fdin);
    }    
    if (exit_var) {
        exit(0);
    }
    if (exit_var) {
        exit(0);
    }
    //Set arrays to NULL.
    memset(inpLine, 0, sizeof inpLine);
    memset(userCmds, 0, sizeof userCmds);
    memset(withParams, 0, sizeof withParams); 
}

int prompt_command(char* args[]) {
    if (args[1] == NULL) {
        printf("current prompt: %s\n", prompt);
        return 0;
    }
    FILE* prompt_file = fopen("prompt.dat", "w"); // open file for writing
    fprintf(prompt_file, "%s", args[1]); // write to file
    fclose(prompt_file); // close file
    printf("Prompt set to %s\n", args[1]); // print to screen
    strcpy(prompt, args[1]); // update prompt
    strcat(prompt, " "); // add space to prompt
    interactiveMode(); // refresh prompt
    return 0;
}
