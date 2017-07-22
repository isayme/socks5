.PHONY: all

CC := gcc

all: libbuffer.a libbuffer.so

libbuffer.a: buffer.o
	ar rcs $@ $^

libbuffer.so: buffer.o
	$(CC) -g -shared -fPIC -Isrc -o $@ $^

test: test_shared test_static

test_shared: libbuffer.so
	rm -rf libbuffer.a
	$(CC) test/test.c -I. -L. -lbuffer -o buffer_test
	./buffer_test
	rm -rf buffer_test

test_static: libbuffer.a
	rm -rf libbuffer.so
	$(CC) test/test.c -I. -L. -lbuffer -o buffer_test
	./buffer_test
	rm -rf buffer_test

clean:
	rm -rf libbuffer.*
	rm -rf buffer_test
