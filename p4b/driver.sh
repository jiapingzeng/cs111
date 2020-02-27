#! /bin/bash

PGM="lab4b"
LOGFILE="test.log"

./$PGM --bogus
ret=$?
if [ $ret -ne 1 ]
then
    printf "Invalid return value for bogus argument: $ret\n"
else
    printf "Passed - Correct return value for bogus argument\n"
fi

p="2"
s="C"
./$PGM --period=$p --scale=$s --log=$LOGFILE <<-EOF
SCALE=F
PERIOD=1
START
STOP
LOG test
OFF
EOF
ret=$?
if [ $ret -ne 0 ]
then
    printf "Return error: $ret\n"
else
    printf "Passed - Correct return value\n"
fi
if [ ! -s $LOGFILE ]
then
    printf "Unable to find log file: $LOGFILE\n"
else
    printf "Passed - Log file successfully created\n"
fi

for c in "SCALE=F" "PERIOD=1" "START" "STOP" "LOG test" "OFF"
do
    grep "$c" $LOGFILE > /dev/null
    if [ $? -ne 0 ]
    then
        printf "Did not log $c command"
    else
        printf "Passed - Successfully logged \"$c\" command\n"
    fi
done
