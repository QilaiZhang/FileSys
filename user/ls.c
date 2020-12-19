#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "fs.h"

void ls(char * filename, char *buf){

    int item_num = fs_read(filename, buf, TYPE_DIR);

    if(item_num == -2){
        char *p = strrchr(filename, '/');
        if(p)
            printf("%s\n", p+1);
        else
            printf("%s\n", filename);
    }

    Dir_item *item = (Dir_item *)buf;

    for(int i = 0; i < item_num; i++){
        if((item + i)->type == TYPE_FILE)
            printf("%s", (item+i)->name);
        else if((item + i)->type == TYPE_DIR)
            printf("\033[1;34m%s\033[0m", (item+i)->name);

        if(i == item_num -1){
            printf("\n");
        }else{
            printf("  ");
        }
    }
}

int main(int argc,char **argv){

    char buf[6 * FS_BLOCKSIZE];

    if(argc < 2){
        ls("/", buf);
        exit(0);
    }

    for(int i=1; i < argc; i++){
        ls(argv[i], buf);
    }

    exit(0);
}