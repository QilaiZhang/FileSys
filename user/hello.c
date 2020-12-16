#include <stdio.h>

int main(int argc,char **argv){
    for(int i=1; i<argc; i++){
        printf("%s ",argv[i]);
        if(i==argc-1)
            printf("\n");
    }
    printf("hello world!\n");
    return 0;
}