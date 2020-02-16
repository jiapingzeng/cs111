#! /bin/bash

test_add()
{
    for i in 10 20 40 80 100 1000 10000; do
        for j in 1 2 4 8 12; do
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
}

test_list()
{
    for i in 10 100 1000 10000 20000; do
        for j in {1..5}; do
            # list-none-none
            ./lab2_list --iterations=$i >> lab2_list.csv
        done
    done

    for i in 1 10 100 1000; do
        for j in 1 2 4 8; do
            for k in {1..5}; do
                # list-none-m
                ./lab2_list --iterations=$i --threads=$j --sync=m >> lab2_list.csv
                # list-none-s
                ./lab2_list --iterations=$i --threads=$j --sync=s >> lab2_list.csv
            done
        done
    done

    for i in 1 2 4 8 16 32; do
        for j in 1 2 4 8 12; do
            # list-none-none
            ./lab2_list --iterations=$i --threads=$j >> lab2_list.csv
            # list-i-none
            ./lab2_list --iterations=$i --threads=$j --yield=i >> lab2_list.csv
            # list-d-none
            ./lab2_list --iterations=$i --threads=$j --yield=d >> lab2_list.csv
            # list-l-none
            ./lab2_list --iterations=$i --threads=$j --yield=l >> lab2_list.csv
            # list-id-none
            ./lab2_list --iterations=$i --threads=$j --yield=id >> lab2_list.csv
            # list-il-none
            ./lab2_list --iterations=$i --threads=$j --yield=il >> lab2_list.csv
            # list-dl-none
            ./lab2_list --iterations=$i --threads=$j --yield=dl >> lab2_list.csv
            # list-idl-none
            ./lab2_list --iterations=$i --threads=$j --yield=idl >> lab2_list.csv
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
            # list-il-m
            ./lab2_list --iterations=$i --threads=$j --sync=m --yield=il >> lab2_list.csv
            # list-dl-m
            ./lab2_list --iterations=$i --threads=$j --sync=m --yield=dl >> lab2_list.csv
            # list-idl-m
            ./lab2_list --iterations=$i --threads=$j --sync=m --yield=idl >> lab2_list.csv
            # list-none-s
            ./lab2_list --iterations=$i --threads=$j --sync=s >> lab2_list.csv
            # list-i-s
            ./lab2_list --iterations=$i --threads=$j --sync=s --yield=i >> lab2_list.csv
            # list-d-s
            ./lab2_list --iterations=$i --threads=$j --sync=s --yield=d >> lab2_list.csv
            # list-l-s
            ./lab2_list --iterations=$i --threads=$j --sync=s --yield=l >> lab2_list.csv
            # list-id-s
            ./lab2_list --iterations=$i --threads=$j --sync=s --yield=id >> lab2_list.csv
            # list-id-s
            ./lab2_list --iterations=$i --threads=$j --sync=s --yield=il >> lab2_list.csv
            # list-dl-s
            ./lab2_list --iterations=$i --threads=$j --sync=s --yield=dl >> lab2_list.csv
            # list-idl-s
            ./lab2_list --iterations=$i --threads=$j --sync=s --yield=idl >> lab2_list.csv
        done
    done
    
    for i in 1 2 4 8 12 16 24; do
        # list-none-none
        ./lab2_list --iterations=1000 --threads=$j >> lab2_list.csv
        # list-none-m
        ./lab2_list --iterations=1000 --threads=$j --sync=m >> lab2_list.csv
        # list-none-s
        ./lab2_list --iterations=1000 --threads=$j --sync=s >> lab2_list.csv
    done
}

rm -f lab2_add.csv
test_add
rm -f lab2_list.csv
test_list
