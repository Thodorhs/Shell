/*Thodorhs Pontzouktzidhs*/
/*csd4336@csd.uoc.gr*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include<stddef.h>
#include <signal.h>
#include <sys/mman.h>
#include <termios.h>

/*color defines for prompt*/
#define BLU  "\x1B[34m"
#define GRN  "\x1B[32m"
#define WHT "\x1B[0m"

/*commands struct*/
struct commandstr{
    char com[150];
    char **args;
    int abort;
    int type;
    int last;
}typedef command;

/*Global aray of commands*/
command *commands;
/*null buffer*/
char * nullbuf=NULL;
/*indicates how many commands found*/
int size;
/*flag set to 1 if dollar character found*/
int dolarfound=0;
/*shared pid of previous child process call after ctrl-z*/
int *lastctrlz;
/*shared pid of child process call*/
int * sharedinchild;

/*display prompt user,cwd*/
void display_prompt(){
char *buff=NULL;
char *usr=getlogin();
char *path=getcwd(buff,1000);
printf("%s%s%s@cs345sh%s%s$",GRN,usr,BLU,path,WHT);
}

/*return string consisting of 'user + cwd + /shell', used in subshell command*/
char * create_path2(){
char*buff=NULL;
char* path = getcwd(buff,1000);
char * path2= (char*)malloc(strlen(path)+6);
strcat(path2,path);
strcat(path2,"/shell");
return path2;
}

/*print contents of commands array including all the values each struct command haves*/
void printc(int size){
int i,j;
printf("size %d\n",size);
 for(i=0;i<size;i++){   
        printf("command: %s pipe=%d, last=%d\n",commands[i].com,commands[i].type,commands[i].last);
        j=0;
        printf("args:\n");
        while(commands[i].args[j]){
            printf("%s\n",commands[i].args[j]);
            j++;
        }
    }
}

/*search for $ in first char of str(str[0])...if found set dolarfound flag and return str without $*/
char *removedol(char *str){
int i,j;
int len = strlen(str);
if(str[0] =='$'){
    for(i=0; i<len; i++){
        if(str[i] =='$'){
        dolarfound=1;   
            /*for each char from $ and on replace with the next one*/
            for(j=i;j<len;j++){
                str[j] =str[j+1];
            }
        len--;
        i--;
        }
    }
}else{
    dolarfound=0;
}
return str;  
}

/*checks if string var(variable until =) haves correct format and returns 1*/
int checkvar(char *var){
int i=0;
    while(var[i]==' '){
            i++;
    }
    while(var[i]!='='){
        if((var[i]<65||var[i]>90)&&(var[i]<97||var[i]>122)&&(var[i]!=95)){
            printf("error: Check var syntax!\n");
            return 0;
        }
    i++;
    }
    return 1;
}

/*checks if = is found in s, return values: 1 found = and variable is in correct form, 2 found = and variable is in incorect form, 0 = not found*/
int checkeq(char* s){
int i=0;
    while(s[i]!='\0'){
        if(s[i]=='='){
            if(checkvar(s)){
                return 1;
            }else{
                return 2;
            }
        }
    i++;
    }
return 0;
}

/*checks line for syntax errors*/
int check_line(char *s){
    int i=0;
    while(s[i]!='\0'){
        if(s[i]==';' && s[i+1]==';'){
            printf("-shell: syntax error near unexpected token ;\n");
            return 1;
        }
    i++;
    }
    return 0;
}

/*append command[index].com extract each argument and store it in command[index].arg[x][] flag=1 means command is in pipe, flag=0 means normal command*/
void attach_args(int flag,int index){
int i=0,j=0,c=0;
int eqcheck;
char *nodol;
char *getemp;
int inquote=0;
char *temp=(char*)malloc(150);/*temp string for storring arg until a special character is found*/
commands[index].args=(char **)calloc(150,sizeof(char *));
    /*remove excess whitespaces*/
    while(commands[index].com[i]==' '){
                i++;
    }
    /*check for variable declare*/
    eqcheck=checkeq(commands[index].com);
    if(eqcheck==2){
        commands[index].abort=1;/*variable declare syntax error abort commands after this command is found (later in main)*/
    }else if(eqcheck==1){/*variable declaration found!!*/
        commands[index].abort=0;/*ok command is legit*/
            while(commands[index].com[i]!='\0'){/*run until \0 found*/
                if(commands[index].com[i]==34&&inquote==0){
                /*if first quote is found set inquote flag*/
                    inquote=1;
                }else if(commands[index].com[i]==34&&inquote==1){
                /*if last quote is found unsetset inquote flag*/
                    inquote=0;
                }
                if(commands[index].com[i]!='=' && commands[index].com[i]!=';'){             
                    /*if com[i] is not = or ; add chars to temp (we are still making the arg!!)*/
                    temp[j]=commands[index].com[i];
                    j++;
                }else{
                    /*ok = or ; found. Arg is ready to be added to arg[c][] but first check if arg haves $ */
                    temp[j]='\0';
                    nodol=removedol(temp);
                    if(dolarfound){
                        /*dollar found!! time for get enviroment variable*/
                        dolarfound=0;/*unset flag for later*/
                        getemp=getenv(nodol);
                        if(getemp!=NULL){
                            /*if enviroment variable value is not null copy it to arg[c] and move to the next one (c++)*/
                            commands[index].args[c]= (char*)malloc(sizeof(getemp));
                            strcpy(commands[index].args[c],getemp);
                            c++;
                        }
                    }else{
                        /*regular arg...str copy and move to the next one (c++) */
                        commands[index].args[c]= (char*)malloc(sizeof(temp));
                        strcpy(commands[index].args[c],temp);
                        c++;
                    }   
                    j=0;/*reinitialize index of temp */
                    /*remove excess whitespaces */
                    while(commands[index].com[i+1]==' '||commands[index].com[i+1]==';'&&inquote==0){
                        i++;
                    }
                    temp =(char*)realloc(temp,150);
                }
                /*if eq is found add it to args and move to the next one (c++) */
                if(commands[index].com[i]=='='){
                    commands[index].args[c]= (char*)malloc(sizeof("="));
                    strcpy(commands[index].args[c],"=");
                    c++;
                }
            i++;
            }
            
    }else{
        commands[index].abort=0;/*ok command is legit*/
            while(commands[index].com[i]!='\0'){/*run until \0 found*/
                if(commands[index].com[i]==34&&inquote==0){
                    /*if first quote is found set inquote flag*/
                    inquote=1;
                }else if(commands[index].com[i]==34&&inquote==1){
                    /*if last quote is found unsetset inquote flag*/
                    inquote=0;
                }
                if(commands[index].com[i]!=' ' && commands[index].com[i]!=';' ||inquote==1){
                    /*if com[i] is not whitespace or ; add chars to temp (we are still making the arg!!)*/         
                    temp[j]=commands[index].com[i];
                    j++;
                }else{
                    /*ok whitespace or ; found arg is ready to be added to arg[c][] but first check if arg haves $ */
                    temp[j]='\0';
                    
                    nodol=removedol(temp);
                    if(dolarfound){
                        /*dollar found!! time for get enviroment variable*/
                        dolarfound=0;
                        getemp=getenv(nodol);
                        if(getemp!=NULL){
                             /*if enviroment variable value is not null copy it to arg[c] and move to the next one (c++)*/
                            commands[index].args[c]= (char*)malloc(sizeof(getemp));
                            strcpy(commands[index].args[c],getemp);
                            c++;
                        }
                    }else{
                        /*regular arg...str copy and move to the next one (c++) */
                        commands[index].args[c]= (char*)malloc(sizeof(temp));
                        strcpy(commands[index].args[c],temp);
                        c++;
                    }   
                    j=0;/*reinitialize index of temp */
                    /*remove excess whitespaces */
                    while(commands[index].com[i+1]==' '||commands[index].com[i+1]==';'&&inquote==0){
                        i++;
                    }
                    temp =(char*)realloc(temp,150);
                }
            i++;
            }
    }
}

/*breaks line to commands, creates the com field of each struct command and returns how many commands where found*/
int break_command(char*line){
int index=0,j=0,i=0;
int size;
/*first check syntax of line*/
if(check_line(line)){
return 0;
}
/*count commands (need this for malloc)*/
while(line[j]){
    if(line[j]==';' || line[j]=='|'|| line[j]=='\n'){
        index++;
        if(line[j]==';'&& line[j+1]=='\n'){
            /*if this is the last arg*/
            break;
        }
    }
j++;
}
/*size=how many commands found, malloc, index=0 to start making args of each command */
size=index;
commands=(command*) malloc(sizeof(command)*index);
index=0;

j=0;
/*start making com field of each command*/
while(line[i]){
    if(line[i]==';'||line[i]=='\n'||line[i]=='\r'){
        /*ok end of command found update fields for pipe (search if this the last command in pipes) */
        if(index!=0){
            if(commands[index-1].type==1){
                /*if not first command and previous is pipe this is the last command is pipe  */
                commands[index].type=1;
                commands[index].last=1;
            }else{
                 /*if not first command and previous is not pipe this is regular command*/
                commands[index].last=0;
                commands[index].type=0;
            }
        }else{
            /*this is regular command the first command cant be pipe only if we are infront of the first the previous commands can be pipes*/
            commands[index].type=0;
        }
        /*need to add these for attach_args*/
        commands[index].com[j]=';';
        commands[index].com[j+1]='\0';
        j=0;
        attach_args(0,index);
        if(checkeq(commands[index].com)==1){
            setenv(commands[index].args[0],commands[index].args[2],1);
        }
        index++;
    }else if(line[i]=='|'){ /*DISCLAIMER: piped commands are being parsed like regular commands with ;,\0 at the end except they are of type=1*/
        commands[index].type=1;
        commands[index].com[j]=';';
        commands[index].com[j+1]='\0';
        j=0;
        attach_args(1,index);
        if(checkeq(commands[index].com)==1){
            setenv(commands[index].args[0],commands[index].args[2],1);
        }
        index++;
    }else{
        commands[index].com[j]=line[i];
        j++;
    }
i++;
}
return size;
}

/*my handler for ctrl_z signal*/
void handle_ctrl_Z(int sig){
    if(*sharedinchild!=0){   
        kill(*sharedinchild,SIGSTOP);
    }else{
        printf("received ctrl-Z signal while in %d parent process nothing to stop... \n",*sharedinchild);
    }
}

/*if pipe is detected this function is called and then returns the index of the next command after the pipe commands*/
int mypipe(int index){
/*fd[0] - read*/
/*fd[1] - write*/
int fd[2];
/*same as main but read DISCLAIMERS later*/
char * path2=create_path2();
int id;
/*DISCLAIMER: this is shared between processes*/
int read=STDOUT_FILENO;
while(1){
    if(strcmp(commands[index].args[0],"exit")==0){
        return -1;/*DISCLAIMER: -1 means exit command found in pipe*/
    }
    if(strcmp(commands[index].args[0],"cd")==0){
        if(commands[index].args[1]){
            chdir(commands[index].args[1]);
            index++;
            continue;
        }
    }else if(commands[index].args[1]!=NULL&&strcmp(commands[index].args[1],"=")==0){
            setenv(commands[index].args[0],commands[index].args[2],1);
            index++;
            continue;
    }else{
    pipe(fd);
    id=fork();
        if(id>0){
            waitpid(-1, NULL, 0);
            /*DISCLAIMER: closing write fd[1] and storing read fd to shared read variable for next command*/
            close(fd[1]);
            read=fd[0];
            if(commands[index].last==1){
                /*DISCLAIMER: if this was the last command we are done!*/
                break;
            }
            index++; /*DISCLAIMER: ok! next command...*/
        }else if(id==0){
            /*DISCLAIMER: making this command's input(stdin) to be the output(stdout)-read-fd[0] of previous command*/
            dup2(read,STDIN_FILENO);
            if(commands[index].last!=1){
                /*DISCLAIMER: if this is not the last command then make output(stdout) of this command to point to the write point of the pipe */
                dup2(fd[1],STDOUT_FILENO);
            }
            /*ok! ready to execute*/
            if(strcmp(commands[index].args[0],"bash")==0){
                execv(path2,&nullbuf);
            }else{
                execvp(commands[index].args[0],commands[index].args);                       
            }
            close(fd[0]);
        }else{
            printf("fork error\n");
            exit(0);
        }
    }
}
return index;
}

/*forking and execution starts in main...*/
int main(){
char *line;
size_t s=0;
char* context = NULL;
int i,j;
int pid;
/*store pid of previous child process*/
lastctrlz=mmap(NULL,4,PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS,-1,0);
/*store pid of child process*/
sharedinchild=mmap(NULL,4,PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS,-1,0);
*sharedinchild=0;

/*ctrlq-ctrls*/
struct termios q_s;
tcgetattr(STDOUT_FILENO, &q_s);
q_s.c_iflag |= IXON;
tcsetattr(STDOUT_FILENO, TCSAFLUSH, &q_s);

/*ctrl-z sigaction struct*/
struct sigaction Z;
/*declaring that handler for ctrlz-SIGSTP will be function handle_ctrl_Z*/
Z.sa_handler=&handle_ctrl_Z;
Z.sa_flags=SA_RESTART;
sigaction(SIGTSTP,&Z,NULL);

char * path2=create_path2();
while(1){
    display_prompt();
    getline(&line,&s,stdin);

    size=break_command(line);
    
    for(i=0;i<size;i++){
        /*dont bother executing a messed sequence of commands*/
        if(commands[i].abort==0){
            if(commands[i].args[1]!=NULL&&strcmp(commands[i].args[1],"=")==0){
                setenv(commands[i].args[0],commands[i].args[2],1);
                continue;
            }
            if(strcmp(commands[i].args[0],"exit")==0){
                return 0;
            }
            if(strcmp(commands[i].args[0],"fg")==0){
                kill(*lastctrlz,SIGCONT);/*wake up previous process*/
            }else if(strcmp(commands[i].args[0],"cd")==0){
                if(commands[i].args[1]){
                    chdir(commands[i].args[1]);
                }
            }else if(commands[i].type==1){
                i=mypipe(i);
                if(i==-1){
                    return 0;
                }
                
            }else{
            pid=fork();/*parent: pid>0 , child: pid==0 , error: pid<0*/
                if(pid<0){
                    printf("fork error\n");
                    exit(0);
                }else if(pid>0){
                    waitpid(*sharedinchild, NULL, WUNTRACED);
                    *lastctrlz=*sharedinchild;
                    *sharedinchild=0;
                }else{
                    *sharedinchild=getpid();
                    if(strcmp(commands[i].args[0],"bash")==0){
                        execv(path2,&nullbuf);
                    }else{
                        execvp(commands[i].args[0],commands[i].args);                   
                    }
                }
            }
        }else{
            break;
        }
    }
}
return 0;
}