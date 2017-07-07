CC := gcc

CFLAGS := -g -Wall -O3 -DLINUX
CXXFLAGS := $(CFLAGS)

vpath %.c src

SOURCES := main.c logger.c netutils.c

ssserver: $(SOURCES)
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -g -o $@
	mv $@ build/

clean:
	rm -rf build/*

builddebian:
	docker build -t debian:gcc .

rundebian:
	docker run -v $PWD:/app/ -it --rm debian:gcc bash
