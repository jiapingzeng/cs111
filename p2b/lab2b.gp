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
#       8. lock time per operation (ns)

# general plot parameters
set terminal png
set datafile separator ","

# graph 1
set title "Throughput vs. number of threads"
set xlabel "Number of threads"
set logscale x 2
set ylabel "Throughput"
set logscale y 10
set output "lab2b_1.png"
set key right top

plot \
    "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) title 'Mutex' with linespoints lc rgb 'red', \
    "< grep -e 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) title 'Spin lock' with linespoints lc rgb 'green'

# graph 2
set title "Wait-for-lock time vs. number of threads"
set xlabel "Number of threads"
set logscale x 2
set ylabel "Time"
set logscale y 10
set output "lab2b_2.png"
set key right top

plot \
    "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($8) title 'Mutex lock time' with linespoints lc rgb 'red', \
    "< grep -e 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($7) title 'Mutex completion time' with linespoints lc rgb 'green'

# graph 3
set title "Successful iterations vs. number of threads"
set xlabel "Number of threads"
set logscale x 2
set ylabel "Successful iterations"
set logscale y 10
set output "lab2b_3.png"
set key right top

plot \
    "< grep -e 'list-id-none,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) title 'Unprotected' with points lc rgb 'red', \
    "< grep -e 'list-id-m,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) title 'Mutex' with points lc rgb 'green', \
    "< grep -e 'list-id-s,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) title 'Spin lock' with points lc rgb 'blue'

# graph 4
set title "Throughput vs. number of threads (Mutex)"
set xlabel "Number of threads"
set logscale x 2
set ylabel "Throughput"
set logscale y 10
set output "lab2b_4.png"
set key right top

plot \
    "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) title '1 list' with linespoints lc rgb 'red', \
    "< grep -e 'list-none-m,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(1000000000/($7)) title '4 lists' with linespoints lc rgb 'green', \
    "< grep -e 'list-none-m,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(1000000000/($7)) title '8 lists' with linespoints lc rgb 'blue', \
    "< grep -e 'list-none-m,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(1000000000/($7)) title '16 lists' with linespoints lc rgb 'yellow'

# graph 5
set title "Throughput vs. number of threads (Spin lock)"
set xlabel "Number of threads"
set logscale x 2
set ylabel "Throughput"
set logscale y 10
set output "lab2b_5.png"
set key right top

plot \
    "< grep -e 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) title '1 list' with linespoints lc rgb 'red', \
    "< grep -e 'list-none-s,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(1000000000/($7)) title '4 lists' with linespoints lc rgb 'green', \
    "< grep -e 'list-none-s,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(1000000000/($7)) title '8 lists' with linespoints lc rgb 'blue', \
    "< grep -e 'list-none-s,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(1000000000/($7)) title '16 lists' with linespoints lc rgb 'yellow'