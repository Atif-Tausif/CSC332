CC = gcc
CFLAGS = -pthread
SRC = src/memview.c src/helpers.c
OUT = build/memview

all: $(OUT)

$(OUT): $(SRC)
	mkdir -p build
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

clean:
	rm -rf build/*

.PHONY: all clean