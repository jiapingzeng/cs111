# NAME: Jiaping Zeng
# EMAIL: jiapingzeng@ucla.edu
# ID: 905363270

CC = gcc
CFLAGS = -g -Wall -Wextra

dist_files = lab1b-client.c lab1b-server.c Makefile README

default: lab1b

lab1b: lab1b-client lab1b-server

lab1b-client:
	$(CC) $(CFLAGS) lab1b-client.c -o $@

lab1b-server:
	$(CC) $(CFLAGS) lab1b-server.c -o $@
