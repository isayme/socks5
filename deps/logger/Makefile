.PHONY: all

CC := gcc
SOFLAGS := -g -shared -fPIC -Isrc

liblogger.a: logger.o
	ar rcs $@ $^

liblogger.so: logger.o
	$(CC) $^ $(SOFLAGS) -o $@

test: test_color test_logger_shared test_logger_static

test_color: tests/color_test.c
	$(CC) tests/color_test.c -o color_test
	./color_test
	rm ./color_test

test_logger_shared: tests/logger_test.c liblogger.so
	$(CC) tests/logger_test.c -I./ -L. -llogger -o logger_test
	./logger_test
	rm ./logger_test

test_logger_static: tests/logger_test.c liblogger.a
	$(CC) tests/logger_test.c -I./ -L. -llogger -o logger_test
	./logger_test
	rm ./logger_test

benchmark:
	$(CC) benchmarks/timeformat.c -Ibenchmarks -o timeformat
	./timeformat
	rm ./timeformat

clean:
	rm -rf liblogger.*
