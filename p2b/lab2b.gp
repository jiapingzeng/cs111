#! /usr/bin/gnuplot
#
# purpose:
#        generate data reduction graphs for multi-threaded list project
#
# input: lab2b_list.csv

# general plot parameters
set terminal png
set datafile separator ","

set title "Graph 1"
set xlabel "x"
set logscale x 10
set ylabel "y"
set logscale y 10
set output "lab2b_1.png"

plot \
    "< grep 'list-none-m,' lab2b_list.csv" using ($3):($7) title 'Mutex' with linespoints lc rgb 'red', \
    "< grep 'list-none-s,' lab2b_list.csv" using ($3):($7)/(4*($3)) title 'Spin lock' with linespoints lc rgb 'green'