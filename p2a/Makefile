# NAME: Jiaping Zeng
# EMAIL: jiapingzeng@ucla.edu
# ID: 905363270

CC = gcc
CFLAGS = -g -Wall -Wextra -lpthread

dist_files = SortedList.h SortedList.c lab2_add.c lab2_list.c lab2_add.csv lab2_list.csv Makefile README driver.sh *.png

default: build

build: SortedList.o lab2_add lab2_list

tests: lab2_add.csv

graphs:
	gnuplot lab2_add.gp
	gnuplot lab2_list.gp

dist: lab2a-905363270.tar.gz

clean:
	rm -rf *test* *.o *.out *.tar.gz lab2_add lab2_list *.png *.csv

lab2_add:
	$(CC) lab2_add.c -o $@ $(CFLAGS)

lab2_list:
	$(CC) lab2_list.c SortedList.o -o $@ $(CFLAGS)

SortedList.o:
	$(CC) SortedList.c -c -o $@ $(CFLAGS)

lab2_add.csv:
	bash driver.sh

lab2a-905363270.tar.gz:
	tar -czf $@ $(dist_files)