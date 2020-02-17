# NAME: Jiaping Zeng
# EMAIL: jiapingzeng@ucla.edu
# ID: 905363270

CC = gcc
CFLAGS = -g -Wall -Wextra -lpthread

default: build

build: SortedList.o lab2_list

tests: build
	bash driver.sh

graphs: tests
	gnuplot lab2b.gp

dist: graphs

clean:
	rm -rf *test* *.out *.o *.tar.gz lab2_list

lab2_list:
	$(CC) lab2_list.c SortedList.o -o $@ $(CFLAGS)

SortedList.o:
	$(CC) SortedList.c -c -o $@ $(CFLAGS)