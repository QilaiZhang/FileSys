USER=./user/
SYS=./sys/
CC=gcc
FLAG=-I./include/

shell:
	$(CC) $(USER)shell.c $(SYS)fs.c $(SYS)disk.c -o $(USER)sh $(FLAG)
	clear
	@./sh