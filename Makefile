CC := gcc

CFLAGS := -g -Wall -O3 -DLINUX -DDEBUG
CXXFLAGS := $(CFLAGS)

vpath %.c src

SOURCES := main.c logger.c netutils.c ev.c buffer.c callback.c

ssserver: $(SOURCES)
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -g -o $@
	mv $@ build/

clean:
	rm -rf build/*

builddebian:
	docker build -t debian:gcc .

rundebian:
	docker run --name ss -p 23456:23456 -v ${PWD}:/app/ -it --rm debian:gcc bash
