CC := gcc

CFLAGS := -g -Wall -O3 -DLINUX
CXXFLAGS := $(CFLAGS)

LDFLAGS := -Wl,-rpath,bin,-rpath, -lm -L./ \
	-Ideps/libev-4.24 -lev 	\
	-Ideps/udns-0.4 -ludns	\
	-Ideps/logger -llogger

vpath %.c src

SOURCES := main.c netutils.c buffer.c callback.c socks5.c resolve.c optparser.c help.c

ssserver: $(SOURCES) libev.a libudns.a liblogger.a
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -g -o $@

libev.a:
	cd deps/libev-4.24 && ./configure && make
	cp deps/libev-4.24/.libs/libev.a ./

libudns.a:
	cd deps/udns-0.4 && ./configure && make
	cp deps/udns-0.4/libudns.a ./

liblogger.a:
	cd deps/logger && make liblogger.a
	cp deps/logger/liblogger.a ./

.PHONY: test
test:
	@./test/test.sh

clean:
	rm -rf ssserver
	rm -rf *.a
	rm -rf *.so

builddebian:
	docker build -t debian:gcc .

rundebian:
	docker run --name ss -p 23456:23456 -v ${PWD}:/app/ -it --rm debian:gcc bash
