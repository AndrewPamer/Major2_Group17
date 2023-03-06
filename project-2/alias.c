#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "alias.h"

/*
Builtin, similar to linux alias
alias alias_name='command'
alias l='ls -al'
alias -r alias_name to remove a single alias
alias -c to clear all aliases
Error handling if alias entered incorrectly
Manual page
*/

AliasNode* alias_list = NULL; //initialize the alias list
char* FILENAME;

int clear_aliases() { 
    alias_list = NULL; // clear the alias list
    dump_aliases(); // dump to file to clear the file
    return 0; 
}

int initialize_alias() {
    alias_list = NULL; //initialize the alias list, necessary when adding new aliases to avoid segfault

    char* home = getenv("HOME");
    char* basename = "/.alias.dat";
    FILENAME = malloc(strlen(home) + strlen(basename) + 1);
    strcpy(FILENAME, home);
    strcat(FILENAME, basename);
    FILE *alias_file;
    alias_file = fopen(FILENAME, "r"); // open the file for reading
    if(alias_file == NULL) {
        // create alias file
        alias_file = fopen(FILENAME, "w");
        fclose(alias_file);
        alias_file = fopen(FILENAME, "r"); // reopen file
        return 1; // error opening file
    }
    char buffer[513]; // 513 to allow for trailing null character

    // Aliases are separated by :
    // Each alias is separated by \n
    AliasNode *curr = NULL;
    while(fgets(buffer, sizeof(buffer), alias_file) != NULL) { // read the file line by line
        
        buffer[strcspn(buffer, "\n")] = 0; // remove newline

        // Split the chunk into alias and command
        char *alias = strtok(buffer, ":"); // alias is the first token
        char *command = strtok(NULL, ""); // command is the rest of the string
        if (alias_list == NULL) { // if the list is empty
            alias_list = (AliasNode*) malloc(sizeof(AliasNode)); // create the first node
            curr = alias_list;
            curr->alias = (char*) malloc(sizeof(char) * strlen(alias)); // allocate memory for the alias
            curr->full_command = (char*) malloc(sizeof(char) * strlen(command)); // allocate memory for the full command

            curr->command = (char*) malloc(sizeof(char) * strlen(command)); // allocate memory for the base command
            curr->parameters = (char*) malloc(sizeof(char) * strlen(command)); // allocate memory for the parameters

            strcpy(curr->alias, alias); // copy the alias into the node
            strcpy(curr->full_command, command); // copy the command into the node
            parse_command(curr, command); // parse the command, will populate the parameters and command
        }
        else { // if the list is not empty
            curr->next = (AliasNode*) malloc(sizeof(AliasNode)); // create a new node
            curr = curr->next; // move to the next node
            curr->alias = (char*) malloc(sizeof(char) * strlen(alias)); // allocate memory for the alias
            curr->full_command = (char*) malloc(sizeof(char) * strlen(command)); // allocate memory for the full command
            
            curr->command = (char*) malloc(sizeof(char) * strlen(command)); // allocate memory for the base command
            curr->parameters = (char*) malloc(sizeof(char) * strlen(command)); // allocate memory for the parameters
            strcpy(curr->alias, alias); // copy the alias into the node
            strcpy(curr->full_command, command); // copy the command into the node
            parse_command(curr, command); // parse the command, will populate the parameters and command
        }
    }
    fclose(alias_file); // close the file

    return 0;
}

int parse_command(AliasNode* node, char* command) {
    char* base_command = strtok(command, " "); // this grabs the base command
    char* parameters = (char*) calloc(512, sizeof(char)); // this is the parameters
    char* token = strtok(NULL, " "); // this grabs the first token
    while (token != NULL) { // while there are more parameters
        strcat(parameters, token); // add the next parameter
        strcat(parameters, " "); // add a space to the parameters
        token = strtok(NULL, " "); // get the next parameter
    }
    strcpy(node->command, base_command); // copy the base command into the node
    strcpy(node->parameters, parameters); // copy the parameters into the node
    return 0;
}

AliasNode* getAlias(char *alias) {
    AliasNode *curr = alias_list; 
    while(curr != NULL) { // iterate through the list
        if(strcmp(curr->alias, alias) == 0) { // if the alias matches
            return curr; // return the node
        }
        curr = curr->next;
    }
    return NULL; // if the alias is not found
}

int addAlias(char *args[]) {
    char* alias = strtok(args[1], "="); //alias is the first argument
    char* base_command = strtok(NULL, ""); // this grabs the base command
    char* parameters = (char*) calloc(512, sizeof(char)); // this is the parameters
    char* aliased_command = (char*) calloc(512, sizeof(char)); // this is the full command

    for (int i = 2; args[i]; i++) { // anything past args[1] are parameters
        // add parameters to full parameter string
        strcat(parameters, " "); 
        strcat(parameters, args[i]);
    }
    strcat(aliased_command, base_command); // add the base command to the full command
    strcat(aliased_command, " ");
    strcat(aliased_command, parameters); // add the parameters to the full command

    FILE *alias_file = fopen(FILENAME, "a"); // append to file
    fprintf(alias_file, "%s:%s\n", alias, aliased_command); // write to file
    fclose(alias_file); // close file
    initialize_alias(); // re-initialize the alias list
    return 0;
}

int alias(char *args[]) {
    if (args[1] == NULL) { // if no arguments
    // print all aliases
        AliasNode *curr = alias_list;
        while(curr != NULL) { // iterate through the list
            printf("%s: %s\n", curr->alias, curr->full_command); // print the alias and the full command
            curr = curr->next;
        }
        return 0;
    }

    if (strcmp(args[1], "-c") == 0) { // clear all aliases
        if (args[2] != NULL) { // if extra arguments
            fprintf(stderr, "alias: too many operands\n");
            return 1;
        }
        return clear_aliases();
    }

    if (strcmp(args[1], "-r") == 0) { // remove alias
        if (args[2] == NULL) { // if no alias
            fprintf(stderr, "alias: missing operand\n");
            return 1;
        }
        if (args[3] != NULL) { // if extra arguments
            fprintf(stderr, "alias: too many operands\n");
            return 1;
        }
        return remove_alias(args[2]);
    }

    char* buffer = (char*) malloc(sizeof(args[1])); // buffer for alias, as strtok modifies the string
    strcpy(buffer, args[1]);
    // all the homies hate strtok
    char* alias_name = strtok(buffer, "=");
    AliasNode *alias_node = getAlias(alias_name);

    if (alias_node != NULL) { // alias already exists
        fprintf(stderr, "alias: %s already exists\n", args[1]);
        return 1;
    }
    addAlias(args);
    return 0;
    
}

int remove_alias(char *alias) {
    printf("removing alias %s\n", alias);
    AliasNode *curr = alias_list;
    AliasNode *prev = NULL;
    while (curr != NULL) { // iterate over list
        if (strcmp(curr->alias, alias) == 0) { // if the alias is found
            if (prev == NULL) {
                alias_list = curr->next; // if the alias is the first node, set the list to the next node
            }
            else {
                prev->next = curr->next; // if the alias is not the first node, set the previous node to the next node
            }
            free(curr); // free the node
            break; 
        }
        prev = curr; 
        curr = curr->next;
    }
    dump_aliases(); // dump the aliases to the file
    return 0;
}

int dump_aliases() {
    FILE* alias_file = fopen(FILENAME, "w"); // open the file for writing, clearing it
    AliasNode *curr = alias_list; // iterate over the list
    while (curr != NULL) {
        fprintf(alias_file, "%s:%s\n", curr->alias, curr->full_command); // write the alias and command to the file
        curr = curr->next;
    }
    fclose(alias_file); // close the file
    return 0;
}