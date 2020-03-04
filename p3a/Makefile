CC = gcc
CFLAGS = -g -Wall -Wextra

dist_files = lab3a.c ext2_fs.h Makefile README

default: lab3a

dist: lab3a-905363270.tar.gz

clean: 
	rm -rf test.* a.out *.o *.tar.gz lab3a

lab3a:
	$(CC) lab3a.c -o $@ $(CFLAGS)

lab3a-905363270.tar.gz:
	tar -czf $@ $(dist_files)