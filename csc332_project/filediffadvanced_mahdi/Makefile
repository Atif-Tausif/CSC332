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

install: $(OUT) $(MAN)
	mkdir -p /usr/local/bin
	mkdir -p /usr/local/share/man/man1
	cp $(OUT) /usr/local/bin/filediffadvanced
	cp $(MAN) /usr/local/share/man/man1/$(MAN)
	mandb

uninstall:
	rm -f /usr/local/bin/filediffadvanced
	rm -f /usr/local/share/man/man1/$(MAN)
	mandb

clean:
	rm -rf build/*


clean_test:
	find . -name "*.txt" -type f -delete
	find . -name "*.bin" -type f -delete

.PHONY: all clean test install uninstall clean_test
