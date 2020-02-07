#! /bin/bash

rm -f lab2_add.csv

for i in 10 20 40 80 100 1000 10000 100000; do
    for j in 2 4 8 12; do
        for k in {1..10}; do
            # add-none
            ./lab2_add --iterations=$i --threads=$j >> lab2_add.csv
            # add-m
            ./lab2_add --iterations=$i --threads=$j --sync=m >> lab2_add.csv
            # add-s
            ./lab2_add --iterations=$i --threads=$j --sync=s >> lab2_add.csv
            # add-c
            ./lab2_add --iterations=$i --threads=$j --sync=c >> lab2_add.csv
            # add-yield-none
            ./lab2_add --iterations=$i --threads=$j --yield >> lab2_add.csv
            # add-yield-m
            ./lab2_add --iterations=$i --threads=$j --yield --sync=m >> lab2_add.csv
            # add-yield-s
            ./lab2_add --iterations=$i --threads=$j --yield --sync=s >> lab2_add.csv
            # add-yield-c
            ./lab2_add --iterations=$i --threads=$j --yield --sync=c >> lab2_add.csv
        done
    done
done