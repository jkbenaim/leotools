CC=gcc
CFLAGS=-O0 -ggdb -Wall -Werror
OS=$(shell uname -s)
ifeq ($(OS),Darwin)
	ICONVFLAG=-liconv
endif

.PHONY: all clean
all: leoimginfo ma2d1view

clean:
	rm -f *.o leoimginfo ma2d1view

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

ma2d1view.o: ma2d1view.c
	$(CC) $(CFLAGS) `sdl-config --cflags` -c $^ -o $@

leoimginfo: leogeo.o mfs.o sha1.o main.o
	$(CC) $(ICONVFLAG) $^ -o $@

ma2d1view: yay1.o ma2d1view.o
	$(CC) `sdl-config --libs` $^ -o $@
