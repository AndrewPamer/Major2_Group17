#pragma once
/*Input Modes*/
void interactiveMode(); //interactive Mode
void batchMode();       //batch Mode.

/*Subroutines*/
void split();           //splits input into commands.
void execute(bool isBatch);         //Executes commands

/*Commands*/
int cd(char* path);     //cd command (cd.c)
//exit
//alias
//...
int prompt_command(char* args[]);