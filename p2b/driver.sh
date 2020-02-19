#! /bin/bash

mutex_waits() {
    for i in 1 2 4 8 12 16 24; do
        ./lab2_list --iterations=1000 --threads=$i --sync=m >>lab2b_list.csv
        ./lab2_list --iterations=1000 --threads=$i --sync=s >>lab2b_list.csv
    done
}

partitioned_lists() {
    for i in 1 4 8 12 16; do
        for j in 1 2 4 8 16; do
            ./lab2_list --yield=id --lists=4 --threads=$i --iterations=$j >>lab2b_list.csv
        done
        for j in 10 20 40 80; do
            ./lab2_list --yield=id --lists=4 --threads=$i --iterations=$j --sync=m >>lab2b_list.csv
            ./lab2_list --yield=id --lists=4 --threads=$i --iterations=$j --sync=s >>lab2b_list.csv
        done
    done
}

synced_lists() {
    for i in 1 2 4 8 12; do
        for j in 1 4 8 16; do
            ./lab2_list --iterations=1000 --threads=$i --lists=$j --sync=m >>lab2b_list.csv
            ./lab2_list --iterations=1000 --threads=$i --lists=$j --sync=s >>lab2b_list.csv
        done
    done
}

rm -f lab2b_list.csv
mutex_waits
partitioned_lists
synced_lists
