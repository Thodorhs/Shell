/*Thodorhs Pontzouktzidhs*/
/*csd4336@csd.uoc.gr*/

/*display prompt*/
void display_prompt(); 

/*return string consisting of 'user + cwd + /shell', used in subshell command*/
char * create_path2();

/*print contents of commands array including all the values each struct command haves*/
void printc(int size);

/*search for $ in first char of str(str[0])...if found set dolarfound flag and RETURN str without $*/
char *removedol(char *str);

/*checks if string var(variable until =) haves correct format and RETURNS 1*/
int checkvar(char *var);

/*checks if = is found in s, RETURN values: 1 found = and variable is in correct form, 2 found = and variable is in incorect form, 0 = not found*/
int checkeq(char* s);

/*checks line for syntax errors*/
int check_line(char *s);

/*append command[index].com extract each argument and store it in command[index].arg[x][] flag=1 means command is in pipe, flag=0 means normal command*/
void attach_args(int flag,int index);

/*breaks line to commands, creates the com field of each struct command and RETURNS how many commands where found*/
int break_command(char*line);

/*my handler for ctrl_z signal*/
void handle_ctrl_Z(int sig);

/*if pipe is detected this function is called and then RETURNS the index of the next command after the pipe commands*/
int mypipe(int index)