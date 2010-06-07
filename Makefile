.PHONY: all clean

CC=gcc

CFLAGS=-ggdb -Wextra -Wall -ansi -pedantic

all : nmea_test

nmea_test : nmea_test.o nmea.o
	$(CC) -o $@ $^

clean :
	rm -f *.o nmea_test

%.o : %.c
	$(CC) -o $@ -c $< $(CFLAGS)

