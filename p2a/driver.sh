#! /bin/bash

rm -f lab2_add.csv

for i in 10 20 40 80 100 1000 10000 100000; do
    for j in 2 4 8 12; do
        for k in {1..10}; do
            ./lab2_add --iterations=$i --threads=$j --yield >> lab2_add.csv
            ./lab2_add --iterations=$i --threads=$j >> lab2_add.csv
        done
    done
done