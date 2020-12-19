#include "stdlib.h"
#include "stdio.h"
#include "fs.h"


int main(int argc,char **argv){
    if(argc < 2){
        printf("Usage: mkdir [dirname]\n");
        exit(0);
    }
    for(int i=1; i < argc; i++){
        fs_create(argv[i], TYPE_DIR);
    }
    
    exit(0);
}