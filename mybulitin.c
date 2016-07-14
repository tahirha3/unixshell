/*
 * fsh.c - the Feeble SHell.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "fsh.h"
#include "builtin.h"


int builtin_exit(char **argv)
{
    if (argv[1] && argv[2]) /* i.e. argc >= 2 */ {
        fprintf(stderr, "usage: exit [status]\n");
        fflush(stderr);
        return(1);
    } else if (argv[1]) {
        /* "exit ###" */
        exit(atoi(argv[1]));
    } else {
        /* "exit" with no argument */
        exit(laststatus);
    }
}

int builtin_cd(char **argv)
{
    if (argv[1] && argv[2]) /* i.e. argc >= 2 */ {
        fprintf(stderr, "usage: cd [dir]\n");
        fflush(stderr);
        return(1);
    } else if (argv[1]) {
        /* "cd ###" */
        if(chdir(argv[1])==-1){
            perror("dir");
            return 1;
        }
        return 0;
    } else {
        /* "cd" with no argument */
        if(getenv("HOME")){
            if(chdir(getenv("HOME"))==-1){
                perror("dir");
                return 1;
            }
            return 0;
        }
        return 1;
    }
}