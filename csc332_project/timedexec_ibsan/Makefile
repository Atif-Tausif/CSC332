CC = gcc
CFLAGS = -O2

SRC = src/finaltimedexec.c
OUT = build/timedexec
MAN = timedexec.1
TEST = tests/timedexec_tests.sh

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
	rm -f tests/*.txt tests/*.bin

.PHONY: all test clean clean_test