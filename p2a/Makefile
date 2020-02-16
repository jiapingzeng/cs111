# NAME: Jiaping Zeng
# EMAIL: jiapingzeng@ucla.edu
# ID: 905363270

CC = gcc
CFLAGS = -g -Wall -Wextra -lpthread

sources = lab2_add.c lab2_list.c SortedList.c SortedList.h
data = lab2_add.csv lab2_list.csv
graphs = lab2_add-1.png lab2_add-2.png lab2_add-3.png lab2_add-4.png lab2_add-5.png lab2_list-1.png lab2_list-2.png lab2_list-3.png lab2_list-4.png

dist_files = Makefile README driver.sh ${sources} ${data} ${graphs}

default: build

build: SortedList.o lab2_add lab2_list

tests: build
	- bash driver.sh

graphs: tests
	gnuplot lab2_add.gp
	gnuplot lab2_list.gp

dist: graphs lab2a-905363270.tar.gz

clean:
	rm -rf *test* *.o *.out *.tar.gz lab2_add lab2_list *.png *.csv

lab2_add:
	$(CC) lab2_add.c -o $@ $(CFLAGS)

lab2_list:
	$(CC) lab2_list.c SortedList.o -o $@ $(CFLAGS)

SortedList.o:
	$(CC) SortedList.c -c -o $@ $(CFLAGS)

lab2a-905363270.tar.gz:
	tar -czf $@ $(dist_files)