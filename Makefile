.PHONY: all clean clean-all index test

CC=gcc
CFLAGS=-ggdb -Wextra -Wall -ansi -pedantic -O2

STRIP=strip
STRIPFLAGS=-s

all : nmea_test

nmea_test : nmea_test.o nmea.o
	$(CC) -o $@ $^
	$(STRIP) $(STRIPFLAGS) $@

index :
	ctags -f tags *.c *.h

test : nmea_test
	./nmea_test

clean :
	rm -f *.o nmea_test

clean-all : clean
	rm -f tags

%.o : %.c
	$(CC) -o $@ -c $< $(CFLAGS)

