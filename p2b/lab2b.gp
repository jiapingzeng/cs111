#! /usr/bin/gnuplot
#
# purpose:
#        generate data reduction graphs for multi-threaded list project
#
# input: lab2b_list.csv
#       1. test name
#       2. # threads
#       3. # iterations per thread
#       4. # lists
#       5. # operations performed (threads x iterations x (ins + lookup + delete))
#       6. run time (ns)
#       7. run time per operation (ns)

# general plot parameters
set terminal png
set datafile separator ","

# graph 1
set title "Operations per second vs number of threads"
set xlabel "Number of threads"
set logscale x 2
set ylabel "Operations per second"
set logscale y 10
set output "lab2b_1.png"
set key right top

plot \
    "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) title 'Mutex' with linespoints lc rgb 'red', \
    "< grep -e 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) title 'Spin lock' with linespoints lc rgb 'green'

# graph 2
set title ""
set xlabel "Number of threads"
set logscale x 2
set ylabel "Time"