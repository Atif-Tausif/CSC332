CC = gcc
CFLAGS = -O2

SRC = src/filediffadvanced.c
OUT = build/filediffadvanced
MAN = filediffadvanced.1
TEST = tests/filediffadvanced_tests.sh

all: $(OUT)

$(OUT): $(SRC)
	mkdir -p build
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

test: $(OUT)
	chmod +x $(TEST)
	./$(TEST)

clean:
	rm -rf build/*


clean_test:
	find . -name "*.txt" -type f -delete
	find . -name "*.bin" -type f -delete

.PHONY: all clean test install uninstall clean_test
