USER=./user/
SYS=./sys/
CC=gcc
FLAG=-I./include/

SRC=\
	hello\

TARGETLIST:=$(patsubst %.c,%,$(SRC))

all:
	clear
	@for file in $(SRC); do\
		gcc $(USER)$$file.c -o $(USER)_$$file;\
	done
	$(CC) $(USER)shell.c $(SYS)fs.c $(SYS)disk.c -o $(USER)_sh $(FLAG)
	@$(USER)_sh

clean:
	rm $(foreach f, $(SRC), $(USER)_$(f)) $(USER)_sh disk
