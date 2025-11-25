#!/bin/bash

CRYPT=../build/filecrypt

echo "=== CSC 332 filecrypt demo tests ==="

# 0) Check binary
if [ ! -x "$CRYPT" ]; then
    echo "Error: $CRYPT not found or not executable. Run 'make' in the project root first."
    exit 1
fi

echo
echo "[1] Preparing test files..."

# One-line input
echo "hello CSC 332 group project" > input_small.txt

# Multi-line input
cat > input_multi.txt <<EOF
Line one of the file.
Line two is here.
Line three has numbers: 12345.
EOF

# Single key file used for both algorithms
echo "secretkey" > key.txt

echo "Created:"
echo "  - input_small.txt  (one line of text)"
echo "  - input_multi.txt  (three lines of text)"
echo "  - key.txt          (shared key for tests)"
echo

run_test() {
    name="$1"
    enc_cmd="$2"
    dec_cmd="$3"
    orig="$4"
    out="$5"

    echo "---- $name ----"
    echo "Encrypt: $enc_cmd"
    eval "$enc_cmd" || { echo "[FAIL] encryption failed"; echo; return; }

    echo "Decrypt: $dec_cmd"
    eval "$dec_cmd" || { echo "[FAIL] decryption failed"; echo; return; }

    if diff "$orig" "$out" >/dev/null 2>&1; then
        echo "[PASS] decrypted output matches original"
    else
        echo "[FAIL] decrypted output does NOT match original"
    fi
    echo
}

# 1) XOR with key file on small input
run_test \
  "Test 1: XOR on small single-line file (key from file)" \
  "$CRYPT -e -a xor -i input_small.txt -o enc_xor_small.bin -k key.txt" \
  "$CRYPT -d -a xor -i enc_xor_small.bin -o out_xor_small.txt -k key.txt" \
  "input_small.txt" \
  "out_xor_small.txt"

# 2) ROL with key file on multi-line input
run_test \
  "Test 2: ROL on multi-line file (key from file)" \
  "$CRYPT -e -a rol -i input_multi.txt -o enc_rol_multi.bin -k key.txt" \
  "$CRYPT -d -a rol -i enc_rol_multi.bin -o out_rol_multi.txt -k key.txt" \
  "input_multi.txt" \
  "out_rol_multi.txt"

# 3) XOR with prompt key (shows -P usage)
echo "---- Test 3: XOR using prompt key (-P) on small file ----"
echo "You will be asked for a key twice."
echo "Type the SAME key both times to pass the test."
echo

echo "Encrypting with prompt key..."
$CRYPT -e -a xor -i input_small.txt -o enc_xor_prompt.bin -P

echo "Decrypting with prompt key..."
$CRYPT -d -a xor -i enc_xor_prompt.bin -o out_xor_prompt.txt -P

if diff input_small.txt out_xor_prompt.txt >/dev/null 2>&1; then
    echo "[PASS] Test 3: prompt key round-trip matches original"
else
    echo "[FAIL] Test 3: prompt key round-trip failed"
fi
echo

echo "Cleaning up temporary encrypted/decrypted files..."
rm -f enc_xor_small.bin out_xor_small.txt \
      enc_rol_multi.bin out_rol_multi.txt \
      enc_xor_prompt.bin out_xor_prompt.txt

echo
echo "Kept:"
echo "  input_small.txt  (for demo)"
echo "  input_multi.txt  (for demo)"
echo "  key.txt          (for demo)"
echo
echo "=== All demo tests finished ==="
