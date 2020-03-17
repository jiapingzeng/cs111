# NAME: Jiaping Zeng
# EMAIL: jiapingzeng@ucla.edu
# ID: 905363270

CC = gcc
CFLAGS = -g -Wall -Wextra -lmraa -lm

dist_files = lab4c_tcp.c lab4c_tls.c Makefile README

default: lab4c_tcp # lab4c_tls

clean:
	rm -rf *test* a.out *.o *.tar.gz lab4c_tcp lab4c_tls

dist: lab4c-905363270.tar.gz

lab4c_tcp:
	$(CC) lab4c_tcp.c -o $@ $(CFLAGS)

lab4c_tls:
	$(CC) lab4c_tls.c -o $@ $(CFLAGS)

lab4c-905363270.tar.gz:
	tar -czf $@ $(dist_files)
