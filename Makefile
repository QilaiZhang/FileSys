USER=./user/
SYS=./sys/
CC=gcc
FLAG=-I./include/

SRC=\
	hello\
	touch\
	mkdir\
	ls\
	cp\

TARGETLIST:=$(patsubst %.c,%,$(SRC))

all:
	clear
	@for file in $(SRC); do\
		gcc $(USER)$$file.c $(SYS)fs.c $(SYS)disk.c -o $(USER)_$$file $(FLAG);\
	done
	@$(CC) $(USER)shell.c $(SYS)fs.c $(SYS)disk.c -o $(USER)_sh $(FLAG)
	@$(USER)_sh

clean:
	rm $(foreach f, $(SRC), $(USER)_$(f)) $(USER)_sh disk
