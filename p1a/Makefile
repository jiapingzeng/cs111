# NAME: Jiaping Zeng
# EMAIL: jiapingzeng@ucla.edu
# ID: 905363270

CC = gcc
CFLAGS = -g -Wall -Wextra

dist_files = lab1a.c Makefile README

default: lab1a

clean:
	rm -rf *test* *.out *.tar.gz lab1a

dist: lab1a-905363270.tar.gz

lab1a:
	$(CC) $(CFLAGS) lab1a.c -o $@

lab1a-905363270.tar.gz:
	tar -czf $@ $(dist_files)