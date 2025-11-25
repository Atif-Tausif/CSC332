#!/bin/bash
echo "---------------Testing Memview---------------"
echo ""
echo "Test 1: Help Message"
../build/memview -h
echo ""
echo "Test 2: System memory info"
../build/memview -s
echo ""
echo "Test 3: Current shell process"
../build/memview -p $$
echo ""
echo "Test 4: Memory maps of current shell"
../build/memview -p $$ -m
echo ""
echo "Test 5: All options together"
../build/memview -p $$ -m -s -S
echo "Test 6: Threading mode"
../build/memview -p $$ -s -t
echo "---------------All Normal Tests Complete---------------"
