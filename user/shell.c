#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "user.h"

#define MAXBUF 500
#define MAXARGS 10
#define MAXPATH 30
#define PATH "./user/_"

char buf[MAXBUF];
char *argv[MAXARGS];
char *whitespace = " ";
char path[MAXPATH] = PATH;
char *p = path + sizeof(PATH) - 1;

void getcmd(char *buf){
    printf("$ ");
    fgets(buf,MAXBUF,stdin);
}

int parsecmd(char *s, char **args)
{
    int cnt = 0;
    while(1)
    {
        while(strchr(whitespace, *s) && *s != '\n')   // parse whitespace
            s++;
        if(*s == '\n')
            break;
        *args = s;
        args++;
        cnt++;
        while(!strchr(whitespace, *s) && *s != '\n')  // parse string
            s++;
        if(*s == '\n')
            break;
        *s = 0;
        s++;
    }
    *s = 0;
    *args = 0;
    return cnt;
}

void runcmd(char **argv, int argc)
{
    argv[argc] = 0;
    strcpy(p, argv[0]);
    execv(path, argv);
    printf("exec %s failed\n", argv[0]);
    exit(-1);
}

int main(){

    int argc;
    while(1){
        getcmd(buf);
        argc = parsecmd(buf, argv);
        if(strcmp(argv[0], "shutdown") == 0){
            printf("File System shutdown!\n\n");
            exit(0);
        }

        if(fork()==0){
            runcmd(argv, argc);
            exit(0);
        }
        wait(0);
    }
    exit(0);
}