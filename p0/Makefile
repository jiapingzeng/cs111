CC = gcc
CFLAGS = -g -Wall -Wextra

dist_files = main.c Makefile README

default: lab0
	
lab0:
	$(CC) $(CFLAGS) main.c -o $@

dist: lab0-905363270.tar.gz

lab0-905363270.tar.gz:
	tar -czf $@ $(dist_files)

clean:
	rm -f *.tgz lab0