CC = gcc
CFLAGS = -Wall -Wextra -pthread

SRC = src/loganalyzer.c
OUT = build/loganalyzer

all: $(OUT)

$(OUT): $(SRC)
	mkdir -p build
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

test: all
	cd test && ./test_loganalyzer.sh

clean:
	rm -rf build/*

.PHONY: all clean test

