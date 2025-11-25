#!/bin/bash

LOGFILE="../log_files.txt"
PROGRAM="../build/loganalyzer"

echo "======================================="
echo " Test 1: Basic Run "
echo "======================================="
$PROGRAM -f "$LOGFILE"
echo

echo "======================================="
echo " Test 2: Pattern Search - 'admin' "
echo "======================================="
$PROGRAM -f "$LOGFILE" -s admin
echo

echo "======================================="
echo " Test 3: Only ERROR Lines "
echo "======================================="
$PROGRAM -f "$LOGFILE" -E
echo

echo "======================================="
echo " Test 4: Only WARNING Lines (with -p) "
echo "======================================="
$PROGRAM -f "$LOGFILE" -W -p
echo

echo "======================================="
echo " Test 5: Date Filter ON (2025-03-01) "
echo "======================================="
$PROGRAM -f "$LOGFILE" -d 2025-03-01
echo

echo "======================================="
echo " Test 6: Date BEFORE (2025-03-01) with -p "
echo "======================================="
$PROGRAM -f "$LOGFILE" -b 2025-03-01 -p
echo

echo "======================================="
echo " Test 7: Date AFTER (2025-03-01) with -p "
echo "======================================="
$PROGRAM -f "$LOGFILE" -a 2025-03-01 -p
echo

echo "======================================="
echo " Test 8: Pattern + Print Lines (ERROR only) "
echo "======================================="
$PROGRAM -f "$LOGFILE" -E -p -s ERROR
echo

echo "======================================="
echo " Error Test 1: Missing -f Option "
echo "======================================="
$PROGRAM
echo

echo "======================================="
echo " Error Test 2: File Does Not Exist "
echo "======================================="
$PROGRAM -f does_not_exist.txt
echo

echo "======================================="
echo " Error Test 3: Invalid Option "
echo "======================================="
$PROGRAM -z
echo

echo "======================================="
echo " All tests completed. "
echo "======================================="

