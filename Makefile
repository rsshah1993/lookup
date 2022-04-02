.PHONY: clean

CC      = gcc
CFLAGS  = -Wall -Wextra 
RM      = rm -f

build: clean
	$(CC) $(CFLAGS) -o build/map src/hash_table.c

clean:
	rm -rf build/
	mkdir -p build/
	touch build/.gitkeep