# NAME: Jiaping Zeng
# EMAIL: jiapingzeng@ucla.edu
# ID: 905363270

CC = gcc
CFLAGS = -g -Wall -Wextra -lmraa -lm

dist_files = *.c Makefile README

default: lab4b

check: lab4b
	./lab4b

clean:
	rm -rf *test* a.out *.o *.tar.gz lab4b

dist: lab4b-905363270.tar.gz

lab4b:
	$(CC) *.c -o $@ $(CFLAGS)

lab4b-905363270.tar.gz:
	tar -czf $@ $(dist_files)
