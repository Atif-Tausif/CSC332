#!/bin/bash

echo " TEST 1: TIME LIMIT EXCEEDED"
echo " Command: ./timedexec -t 2 -- sleep 10"
echo " Expected: Child process should be killed after 2 seconds"

./timedexec -t 2 -- sleep 10
echo
echo


echo " TEST 2: NORMAL EXECUTION"
echo " Command: ./timedexec -t 5 -- ls"
echo " Expected: Program should finish normally before timeout"

./timedexec -t 5 -- ls
echo
echo


echo " TEST 3: EXEC FAILURE"
echo " Command: ./timedexec -t 5 -- does_not_exist"
echo " Expected: Should print execvp error and exit with code 127"

./timedexec -t 5 -- does_not_exist
echo
echo



echo " TEST 4: MISSING REQUIRED ARGUMENT"
echo " Command: ./timedexec -- ls"
echo " Expected: Should show usage message because -t is missing"

./timedexec -- ls
echo
echo


 
echo " TEST 5: CTRL-C INTERRUPT (USER MUST PRESS CTRL-C)"
echo " Command: ./timedexec -t 20 -- sleep 30"
echo " Expected: Press Ctrl-C → parent kills child and prints message"
 
./timedexec -t 20 -- sleep 30
echo
echo