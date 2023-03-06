#pragma once

typedef struct AliasNode {
    char *alias;
    char *command;
    char *full_command;
    char *parameters;
    struct AliasNode *next;   
} AliasNode;

AliasNode* getAlias(char *alias);
int initialize_alias();
int alias(char *args[]);
int clear_aliases();
int remove_alias(char* alias);
int dump_aliases();
int parse_command(AliasNode* node, char* command);