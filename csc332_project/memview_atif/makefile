CC = gcc
CFLAGS = -pthread
SRC = src/memview.c src/helpers.c
OUT = build/memview

all: $(OUT)

$(OUT): $(SRC)
	mkdir -p build
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

tests: $(OUT)
	@echo "         Running memview tests      "
	@echo ""
	@chmod +x tests/test_memview.sh tests/test_errors.sh
	@cd tests && ./test_memview.sh
	@echo ""
	@cd tests && ./test_errors.sh
	@echo ""
	@echo "         All tests completed      "

clean:
	rm -rf build/*

.PHONY: all clean tests