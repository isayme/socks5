#-----------------------------------------------------------
# general use makefile file.
# for unix like OS, special for c language.
#-----------------------------------------------------------

#-----------------------------------------------------------
# to use this makefile, generally change variables below
#-----------------------------------------------------------
# binarys to be created
BINS := socks5

# libs to be created
LIBS := liblog.so libdaemon.so libthread.so libconfig.so
#-----------------------------------------------------------

# compiler tool
CC := gcc

# compile option to the executables
CFLAGS := -g -Wall -O3 -DLINUX -Iinc
CXXFLAGS := $(CFLAGS)

#compile option to .so
SOFLAGS := -g -DLINUX -shared -fPIC -Iinc

LDFLAGS := -Wl,-rpath,bin,-rpath, \
  -Lbin \
	-lpthread -llog -ldaemon -lconfig -lev -lthread
	
# vpath indicate the searching path of the according file type
SRCDIR := src $(shell ls -d src/*)
vpath %.c $(SRCDIR)
vpath %.h inc
vpath %.so bin
vpath % bin
vpath %.cpp $(SRCDIR)

.PHONY: all libs bins install uninstall

all : libs bins

libs : $(LIBS)

bins : $(BINS)

clean :
	cd bin;\
	rm -f $(LIBS);\
	rm -f $(BINS);\
	cd ..;
		
		
# common rules goes here, if the compiling procedure of your module matches one, 
# no need to list it in SpecialRules
%.so : %.c
	$(CC) $^ $(SOFLAGS) -o $@
	mv $@ bin/

% : %.c
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -o $@
	mv $@ bin/
		
#-----------------------------------------------------------
# for special libs/bins, add some lines like below
#-----------------------------------------------------------
#libso_example.so: so_example.c so_prerequisite1.c so_prerequisite2.c
#	$(CC) $(SOFLAGS) -o $@ $^
#	mv $@ bin/

#bin_example : bin_example.c bin_prerequisite1.c bin_prerequisite2.c
#	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
#	mv $@ bin/
#-----------------------------------------------------------


