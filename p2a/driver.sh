#! /bin/bash

test_add()
{
    for i in 10 20 40 80 100 1000 10000; do
        for j in 1 2 4 8 12; do
            for k in {1..5}; do
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
}

test_list()
{
    for i in 1 2 4 8 16 32; do
        for j in 2 4 8 12; do
            for k in {1..5}; do
                # list-none-m
                ./lab2_list --iterations=$i --threads=$j --sync=m >> lab2_list.csv
                # list-i-m
                ./lab2_list --iterations=$i --threads=$j --sync=m --yield=i >> lab2_list.csv
                # list-d-m
                ./lab2_list --iterations=$i --threads=$j --sync=m --yield=d >> lab2_list.csv
                # list-l-m
                ./lab2_list --iterations=$i --threads=$j --sync=m --yield=l >> lab2_list.csv
                # list-id-m
                ./lab2_list --iterations=$i --threads=$j --sync=m --yield=id >> lab2_list.csv
                # list-dl-m
                ./lab2_list --iterations=$i --threads=$j --sync=m --yield=dl >> lab2_list.csv
                # list-idl-m
                ./lab2_list --iterations=$i --threads=$j --sync=m --yield=idl >> lab2_list.csv
            done
        done
    done
    
    for i in 1 2 4 8 12 16 24; do
        ./lab2_list --iterations=1000 --threads=$j >> lab2_list.csv
        ./lab2_list --iterations=1000 --threads=$j --sync=m >> lab2_list.csv
        ./lab2_list --iterations=1000 --threads=$j --sync=s >> lab2_list.csv
    done
}

rm -f lab2_add.csv lab2_list.csv
test_add
test_list
