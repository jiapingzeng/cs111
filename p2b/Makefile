# NAME: Jiaping Zeng
# EMAIL: jiapingzeng@ucla.edu
# ID: 905363270

CC = gcc
CFLAGS = -g -Wall -Wextra -lpthread -lprofiler

sources = lab2_list.c SortedList.c SortedList.h
data = lab2b_list.csv
graphs = lab2b_1.png lab2b_2.png lab2b_3.png lab2b_4.png lab2b_5.png
profile = profile.out

dist_files = Makefile README driver.sh lab2b.gp ${sources} ${data} ${graphs} ${profile}

default: build

build: SortedList.o lab2_list

tests: build
	bash driver.sh

profile:

graphs: tests
	gnuplot lab2b.gp

dist: graphs lab2b-905363270.tar.gz

clean:
	rm -rf *test* a.out *.o *.tar.gz lab2_list

lab2_list:
	$(CC) lab2_list.c SortedList.o -o $@ $(CFLAGS)

SortedList.o:
	$(CC) SortedList.c -c -o $@ $(CFLAGS)

lab2b-905363270.tar.gz:
	tar -czf $@ $(dist_files)