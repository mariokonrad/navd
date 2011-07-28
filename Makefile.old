.PHONY: all clean index test valgrind

CC=gcc
CFLAGS=-ggdb -Wextra -Wall -pedantic -std=c99

STRIP=strip
STRIPFLAGS=-s

SENTENCES=\
	src/nmea0183/nmea_sentence_gprmb.o \
	src/nmea0183/nmea_sentence_gprmc.o \
	src/nmea0183/nmea_sentence_gpgga.o \
	src/nmea0183/nmea_sentence_gpgsv.o \
	src/nmea0183/nmea_sentence_gpgsa.o \
	src/nmea0183/nmea_sentence_gpgll.o \
	src/nmea0183/nmea_sentence_gpbod.o \
	src/nmea0183/nmea_sentence_gpvtg.o \
	src/nmea0183/nmea_sentence_gprte.o \
	src/nmea0183/nmea_sentence_pgrme.o \
	src/nmea0183/nmea_sentence_pgrmm.o \
	src/nmea0183/nmea_sentence_pgrmz.o \
	src/nmea0183/nmea_sentence_hchdg.o

all : nmea_test dump

nmea_test : nmea_test.o src/nmea0183/nmea_util.o $(SENTENCES) src/nmea0183/nmea.o
	$(CC) -o $@ $^
#	$(STRIP) $(STRIPFLAGS) $@

dump : dump.o src/nmea0183/nmea.o src/nmea0183/nmea_util.o $(SENTENCES)
	$(CC) -o $@ $^
#	$(STRIP) $(STRIPFLAGS) $@

valgrind : dump nmea_test
	valgrind -v ./dump
#	valgrind -v ./nmea_test

index :
	ctags --recurse -f tags *.c src/*

test : nmea_test
	./nmea_test

#sqlite3.o : sqlite3.c sqlite3.h sqlite3ext.h
#	$(CC) -o $@ -c sqlite3.c -I. -O2 -DSQLITE_OMIT_LOAD_EXTENSION=1 -DSQLITE_THREADSAFE=0

clean :
	rm -f *.o nmea_test dump
	rm -f src/nmea0183/*.o
	rm -f tags

%.o : %.c
	$(CC) -o $@ -c $< $(CFLAGS) -Isrc

