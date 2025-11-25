CC = gcc
CFLAGS = -O2

SRC = src/filecrypt.c
OUT = build/filecrypt

all: $(OUT)

$(OUT): $(SRC)
	mkdir -p build
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

run: $(OUT)
	./$(OUT)

clean:
	rm -rf build/*

.PHONY: all run clean