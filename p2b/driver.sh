#! /bin/bash

rm -f lab2b_list.csv

for i in 1 2 4 8 12 16 24; do
    #for j in {1..10}; do
        ./lab2_list --iterations=1000 --threads=$i --sync=m >> lab2b_list.csv
        ./lab2_list --iterations=1000 --threads=$i --sync=s >> lab2b_list.csv
    #done
done