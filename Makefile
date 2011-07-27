.PHONY: all clean clean-all index test valgrind

CC=gcc
CFLAGS=-ggdb -Wextra -Wall -pedantic -std=c99

STRIP=strip
STRIPFLAGS=-s

SENTENCES=\
	nmea_sentence_gprmb.o \
	nmea_sentence_gprmc.o \
	nmea_sentence_gpgga.o \
	nmea_sentence_gpgsv.o \
	nmea_sentence_gpgsa.o \
	nmea_sentence_gpgll.o \
	nmea_sentence_gpbod.o \
	nmea_sentence_gpvtg.o \
	nmea_sentence_gprte.o \
	nmea_sentence_pgrme.o \
	nmea_sentence_pgrmm.o \
	nmea_sentence_pgrmz.o \
	nmea_sentence_hchdg.o

all : nmea_test dump

nmea_test : nmea_test.o nmea_util.o $(SENTENCES) nmea.o
	$(CC) -o $@ $^
#	$(STRIP) $(STRIPFLAGS) $@

dump : dump.o nmea.o nmea_util.o $(SENTENCES)
	$(CC) -o $@ $^
#	$(STRIP) $(STRIPFLAGS) $@

valgrind : dump nmea_test
	valgrind -v ./dump
#	valgrind -v ./nmea_test

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

