# NAME: Jiaping Zeng
# EMAIL: jiapingzeng@ucla.edu
# ID: 905363270

SHELL := /bin/bash
CC = gcc
CFLAGS = -g -Wall -Wextra -lpthread

dist_files = lab2_add.c lab2_list.c

default: build

build: lab2_add

tests: lab2_add.csv

lab2_add:
	$(CC) lab2_add.c -o $@ $(CFLAGS)

lab2_list:
	$(CC) lab2_list.c -o $@ $(CFLAGS)

lab2_add.csv:
	rm -f lab2_add.csv ; \
	for i in 10 20 40 80 100 1000 10000 100000; do \
    	for j in 2 4 8 12; do \
        	for k in {1..10}; do \
            	./lab2_add --iterations=$i --threads=$j >> lab2_add.csv ; \
    	        ./lab2_add --iterations=$i --threads=$j --sync=m >> lab2_add.csv ; \
            	./lab2_add --iterations=$i --threads=$j --sync=s >> lab2_add.csv ; \
    	        ./lab2_add --iterations=$i --threads=$j --sync=c >> lab2_add.csv ; \
            	./lab2_add --iterations=$i --threads=$j --yield >> lab2_add.csv ; \
    	        ./lab2_add --iterations=$i --threads=$j --yield --sync=m >> lab2_add.csv ; \
            	./lab2_add --iterations=$i --threads=$j --yield --sync=s >> lab2_add.csv ; \
    	        ./lab2_add --iterations=$i --threads=$j --yield --sync=c >> lab2_add.csv ; \
        	done ; \
	    done ; \
	done