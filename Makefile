.PHONY: all clean

CC=gcc

CFLAGS=-Wextra -Wall -ansi -pedantic

all : nmea

nmea : nmea.o
	$(CC) -o $@ $^

clean :
	rm -f *.o nmea

%.o : %.c
	$(CC) -o $@ -c $< $(CFLAGS)

