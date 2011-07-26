.PHONY: all clean clean-all index test valgrind

CC=gcc
CFLAGS=-ggdb -Wextra -Wall -ansi -pedantic -O2

STRIP=strip
STRIPFLAGS=-s

all : nmea_test dump

nmea_test : nmea_test.o nmea.o
	$(CC) -o $@ $^
	$(STRIP) $(STRIPFLAGS) $@

dump : dump.o nmea.o
	$(CC) -o $@ $^
#	$(STRIP) $(STRIPFLAGS) $@

valgrind : dump
	valgrind -v ./dump

index :
	ctags -f tags *.c *.h

test : nmea_test
	./nmea_test

#sqlite3.o : sqlite3.c sqlite3.h sqlite3ext.h
#	$(CC) -o $@ -c sqlite3.c -I. -O2 -DSQLITE_OMIT_LOAD_EXTENSION=1 -DSQLITE_THREADSAFE=0

clean :
	rm -f *.o nmea_test dump

clean-all : clean
	rm -f tags

%.o : %.c
	$(CC) -o $@ -c $< $(CFLAGS) -I.

