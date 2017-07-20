CC := gcc

CFLAGS := -g -Wall -O3 -DLINUX
CXXFLAGS := $(CFLAGS)

LDFLAGS := -Wl,-rpath,bin,-rpath, -lm -L./ -Ideps/libev-4.24 -lev -Ideps/udns-0.4 -ludns

vpath %.c src

SOURCES := main.c logger.c netutils.c buffer.c callback.c socks5.c resolve.c

ssserver: $(SOURCES) libev.a libudns.a
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -g -o $@

libev.a:
	cd deps/libev-4.24 && ./configure && make
	cp deps/libev-4.24/.libs/libev.a ./

libudns.a:
	cd deps/udns-0.4 && ./configure && make
	cp deps/udns-0.4/libudns.a ./

.PHONY: test
test:
	@./test/test.sh

clean:
	rm -rf ssserver
	rm -rf libev.*
	rm -rf libudns.*

builddebian:
	docker build -t debian:gcc .

rundebian:
	docker run --name ss -p 23456:23456 -v ${PWD}:/app/ -it --rm debian:gcc bash
