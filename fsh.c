/*
 * fsh.c - the Feeble SHell.
 */

#include <stdio.h>
#include "fsh.h"
#include "parse.h"
#include "error.h"

int showprompt = 1;
int laststatus = 0;  /* set by anything which runs a command */


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
    for (; p; p = p->next) {
        if (p->inputfile)
            printf("redirecting input from %s\n", p->inputfile);
        if (p->outputfile)
            printf("redirecting output to %s\n", p->outputfile);
        if (p->pl)
            printf("executing %s...\n", p->pl->argv[0]);
    }
}
