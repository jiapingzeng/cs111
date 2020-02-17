#! /bin/bash

rm -f lab2.csv

for i in 1 2 4 8 12 16 24; do
    for j in {1..5}; do
        ./lab2_list --iterations=1000 --threads=$i --sync=m >> lab2b.csv
        ./lab2_list --iterations=1000 --threads=$i --sync=s >> lab2b.csv
    done
done