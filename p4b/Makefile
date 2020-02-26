# NAME: Jiaping Zeng
# EMAIL: jiapingzeng@ucla.edu
# ID: 905363270

CC = gcc
CFLAGS = -g -Wall -Wextra -lmraa -lm

dist_files = lab4b.c Makefile README

default: lab4b

check: lab4b

clean:
	rm -rf *test* a.out *.o *.tar.gz lab4b

dist: lab4b-905363270.tar.gz

lab4b:
	$(CC) lab4b.c -o $@ $(CFLAGS)

lab4b-905363270.tar.gz:
	tar -czf $@ $(dist_files)
