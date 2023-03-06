# **Major 2**

### **Names**

- Ryan O'Dell
- Andrew Pamer
- Tyrese Palmer
- Julius Ogene

### **Organization of Project**

- `alias`
    - Ryan O'Dell
- `exit`
    - Ryan O'Dell
- `prompt`
    - Ryan O'Dell
- `cd`
    - Andrew Pamer
- `Signal Handling`
    - Andrew Pamer
- `Piping/Redirection`
    - Andrew Pamer
- `Structure of major2.c`
    - Ryan O'Dell, Andrew Pamer
- `Defensive Programming`
    - Ryan O'Dell, Andrew Pamer
- `Makefile`
    - Andrew Pamer
- `README`
    - Ryan O'Dell
- `path`
    - Tyrese Palmer
- `myhistory`
    - Julius Ogene

### **Design Overview**

First, we take the string and split it into the command and arguments, separating them by semicolons if necessary. We can then store the parameters into an array, with the command as array\[0] and all parameters after that. This allows for easy parameter counts as well as comparisons, as each parameter has it's own entry in the array.

From there, first we check to see if it is an alias, and if it is, we "convert" that alias back into a parameterized comand.
After that, we check to see if it is a builtin, and if so, we call the relevant builtin. Finally, if it is none of those, we pass it to `execvp` using our PATH variable and check if it succeeded.

PATH, history, aliases, and the prompt are all stored in a file that is loaded on shell startup, allowing for persistence between sessions. They are all stored at ~/.\[path|history|alias\].dat, These are loaded at the beginning of main, and the files are update whenever they are written to.

### **Complete Specification**

Our shell handles extra whitespace or extra semicolons as expected, it runs the command and continues processing as normal.
The PATH variable defaults to whatever the system PATH variable is, and is updated whenever the user changes it. These changes persist between sessions, and the system PATH variable is not affected.  We've checked all builtins to ensure that they have the minimum required of parameters as to avoid a segfault, and we've done the same with extra parameters. We check return values for different system calls to make sure that they've returned without error and we handle them accordingly if they have. If a command is not found, it will print to stderr and continue processing.

### Known Issues

myhistory
- myhistory -e is not implemented

cd
- N/A

path
- N/A

alias
- N/A