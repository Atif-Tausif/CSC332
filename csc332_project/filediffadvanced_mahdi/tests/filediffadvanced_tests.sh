#!/bin/bash

CMD="./build/filediffadvanced"

echo "Running filediffadvanced test suite"
echo "-------------------------------------"

# Setup test files
echo "hello world" > t1_a.txt
echo "hello world" > t1_b.txt

echo "hello world" > t2_a.txt
echo "hello World" > t2_b.txt

printf "abc" > t3_short.bin
printf "abcXYZ" > t3_long.bin

# --- TEST 1: Identical files ---
echo -e "\nTEST 1: Identical small text files"
$CMD -s t1_a.txt t1_b.txt
echo "Exit code: $?"

# --- TEST 2: One-character difference, text mode ---
echo -e "\nTEST 2: One-character difference"
$CMD -s -t -o 5 t2_a.txt t2_b.txt
echo "Exit code: $?"

# --- TEST 3: Different lengths ---
echo -e "\nTEST 3: Different length files"
$CMD -s t3_short.bin t3_long.bin
echo "Exit code: $?"

# --- TEST 4: Brief identical ---
echo -e "\nTEST 4: Brief identical mode"
$CMD -b t1_a.txt t1_b.txt
echo "Exit code: $?"

# --- TEST 5: Missing arguments ---
echo -e "\nTEST 5: Missing arguments"
$CMD
echo "Exit code: $?"

# --- TEST 6: Non-existent file ---
echo -e "\nTEST 6: Non-existent file"
$CMD no_such_file.txt t1_a.txt
echo "Exit code: $?"

# --- TEST 7: Permission denied ---
echo -e "\nTEST 7: Permission denied"
echo "secret" > no_read.txt
chmod 000 no_read.txt
$CMD no_read.txt t1_a.txt
echo "Exit code: $?"
chmod 644 no_read.txt

# --- TEST 8: SIGINT interruption ---
echo -e "\nTEST 8: SIGINT interruption (Press CTRL+C)"
yes "abcdefghij" | head -n 200000 > big1.txt
cp big1.txt big2.txt
printf "X" | dd of=big2.txt bs=1 seek=0 count=1 conv=notrunc 2>/dev/null
$CMD -s big1.txt big2.txt
echo "Exit code: $?"

echo -e "\nTest suite completed."
