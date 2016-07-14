/*
 * fsh.c - the Feeble SHell.
 */
#include <stdio.h>
#include "fsh.h"
#include "parse.h"
#include "error.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>

int showprompt = 1;
int laststatus = 0;  /* set by anything which runs a command */
extern char **environ;
char *searchList[] = {
        "/bin/",
        "/usr/bin/",
        ""
    };
int getCmd(char *command);
int builtin_exit(char **argv);
int builtin_cd(char **argv);

int main()
{
    char buf[1000];
    struct parsed_line *p;
    extern void execute(struct parsed_line *p);

    while (1) {
        if (showprompt)
            printf("$ ");
        if (fgets(buf, sizeof buf, stdin) == NULL)
            break;
        if ((p = parse(buf))) {
            execute(p);
            freeparse(p);
        }
    }

    return(laststatus);
}

void execute(struct parsed_line *p)
{
    struct parsed_line *cmd;
    struct pipeline *pl;
    int in, out;
    for (cmd = p; cmd; cmd = cmd->next) {
        if (cmd != p) {
            switch (cmd->conntype) {
            case CONN_SEQ:
                //  excecute any way
            break;
            case CONN_AND:
                // skip if laststats is non zero
                // only excute if laststatus zero
                if(laststatus!=0){
                    continue;
                }
            break;
            case CONN_OR:
                // skip if laststats is zero
                // only excute if laststatus is non-zero
                if(laststatus==0){
                    continue;
                }
            break;
            }
        }
        
        if(cmd->pl != NULL){
            if(!strcmp(cmd->pl->argv[0], "exit")){
                laststatus=builtin_exit(cmd->pl->argv);
            }
            if(!strcmp(cmd->pl->argv[0], "cd")){
                laststatus=builtin_cd(cmd->pl->argv);
                continue;
            }
        }

        /*if (cmd->inputfile) {
            in = open(cmd->inputfile,O_RDONLY);
            dup2(in,STDIN_FILENO);
            close(in);
        }*/

        for (pl = cmd->pl; pl; pl = pl->next) {
            if (cmd->pl) {
                char *command;
                int cmdPath=1;
                
                if (strchr(pl->argv[0], '/')){
                    command=pl->argv[0];
                } else {
                    cmdPath=getCmd(pl->argv[0]);
                    if(cmdPath != -1){
                        command = malloc(strlen(searchList[cmdPath]) + strlen(pl->argv[0]) + 1);
                        strcpy(command, searchList[cmdPath]);
                        strcat(command, pl->argv[0]);
                        cmdPath=-2;
                    } else {
                        command=NULL;
                    }
                }
                if(command) {
                    int x = fork();
                    if (x == -1) {
                        perror("fork error");
                        laststatus=1;
                    } else if (x == 0) {
                        /* child */
                        /*if (cmd->inputfile) {
                            close(1);
                            if (open(cmd->inputfile, O_WRONLY|O_CREAT|O_TRUNC, 0666) < 0) {
                                perror("file");
                                laststatus=1;
                            }
                        }*/
                        if (cmd->inputfile) {
                            in = open(cmd->inputfile,O_RDONLY);
                            if(in < 0){
                                perror("file");
                                laststatus=1;
                            }
                            dup2(in,STDIN_FILENO);
                            close(in);
                        }
                        if (cmd->outputfile) {
                            out = open(cmd->outputfile,O_WRONLY|O_CREAT,0666); // Should also be symbolic values for access rights
                            if(out < 0){
                                perror("file");
                                laststatus=1;
                            }
                            dup2(out,STDOUT_FILENO);
                            close(out);
                        }
                        laststatus=execve(command, pl->argv, environ);
                        perror("execve error");
                    } else {
                        /* parent */
                        int status, pid;
                        pid = wait(&status);
                        laststatus=status;
                    }
                }

                if(cmdPath == -2 && command){
                    free(command);
                }
            }
        }

        /*if (cmd->outputfile) {
            out = open(cmd->outputfile,O_WRONLY|O_CREAT,0666); // Should also be symbolic values for access rights
            dup2(out,STDOUT_FILENO);
            close(out);
        }*/
    }
}

/*
 * getCmd function is used to find the path of file
 * to excecute.
 */

int getCmd(char *command)
{
    int i=0;
    for(;i<3;i++){
        static char *buf = NULL;
        static int bufsize = 0;
        int sizeneeded = strlen(searchList[i]) + strlen(command) + 1;
        struct stat statbuf;

        if (sizeneeded > bufsize) {
        if (buf == NULL) {
            bufsize = 500;
        } else {
            bufsize = sizeneeded + 100;
            free(buf);
        }
        if ((buf = malloc(bufsize)) == NULL) {
            fprintf(stderr, "out of memory!\n");  /* very very unlikely */
            exit(1);
        }
        }
        strcpy(buf, searchList[i]);
        strcat(buf, command);
        if (stat(buf, &statbuf) || (statbuf.st_mode & 0100) == 0) {
            // cmd not found
            continue;
        } else {
            // cmd found
            return i;
        }
    }
    printf("%s: Command not found.\n", command);
    laststatus = 1;
    return(-1);
}
