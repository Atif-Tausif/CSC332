#!/bin/bash
echo "--------Error Tests--------"
echo "Test 1: No PID with -m flag"
../build/memview -m
echo ""
echo "Test 2: Invalid PID"
../build/memview -p 999999
echo ""
echo "Test 3: Negative PID"
../build/memview -p -1
echo ""
echo "Test 4: Invalid option"
../build/memview -z
echo ""
echo "Test 5: Maps without PID"
../build/memview -m -s
echo ""
echo "--------All Error Tests Complete--------"