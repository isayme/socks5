CC := gcc

CFLAGS := -g -Wall -O3 -DLINUX -DDEBUG
CXXFLAGS := $(CFLAGS)

LDFLAGS := -Wl,-rpath,bin,-rpath, -L./ -Ideps/libev-4.24 -lev

vpath %.c src

SOURCES := main.c logger.c netutils.c buffer.c callback.c socks5.c

ssserver: $(SOURCES) libev.so
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -g -o $@

libev.so:
	cd deps/libev-4.24 && ./configure && make
	cp deps/libev-4.24/.libs/libev.so ./

clean:
	rm -rf ssserver
	rm -rf libev.*

builddebian:
	docker build -t debian:gcc .

rundebian:
	docker run --name ss -p 23456:23456 -v ${PWD}:/app/ -it --rm debian:gcc bash
