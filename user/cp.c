#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "fs.h"


int main(int argc,char **argv){

    int size;
    char buf[6 * FS_BLOCKSIZE];
    char pathname[121];

    if(argc != 3){
        printf("Usage: cp [filename] [path]\n");
        exit(0);
    }

    size = fs_read(argv[1], buf, TYPE_FILE);
    if(size < 0){
        if(size == -2)
            printf("cp: connot copy a directory.\n");
        exit(0);
    }

    char *p = strrchr(argv[1], '/');
    if(!p) p = argv[1];
    else p++;
    
    strcpy(pathname, argv[2]);
    strcpy(pathname + strlen(pathname) + 1, p);
    pathname[strlen(pathname)] = '/';

    fs_create(pathname, TYPE_FILE);
    
    fs_write(pathname, buf, size);

    exit(0);
}