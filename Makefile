CC=gcc
CFLAGS=-O0 -ggdb -Wall -std=gnu99
OS=$(shell uname -s)
ifeq ($(OS),Darwin)
	ICONVFLAG=-liconv
endif

.PHONY: all clean
all: leoimginfo ma2d1view mathumbview psppmview mfsextract ma3d1view

clean:
	rm -f *.o leoimginfo ma2d1view mathumbview psppmview mfsextract ma3d1view

ma2d1view.o: ma2d1view.c
	$(CC) $(CFLAGS) `sdl-config --cflags` -c $^ -o $@
	
ma3d1view: ma3d1view.c
	$(CC) $(CFLAGS) $^ -o $@ -lX11 -lGL -lGLU -lglut

mathumbview.o: mathumbview.c
	$(CC) $(CFLAGS) `sdl-config --cflags` -c $^ -o $@

psppmview.o: psppmview.c
	$(CC) $(CFLAGS) `sdl-config --cflags` -c $^ -o $@
	
mfsextract: mfsextract.o mfs.o leogeo.o

leoimginfo: leogeo.o mfs.o sha1.o leoimginfo.o
	$(CC) $(ICONVFLAG) $^ -o $@

ma2d1view: yay1.o ma2d1view.o
	$(CC) `sdl-config --libs` $^ -o $@

mathumbview: mathumbview.o
	$(CC) `sdl-config --libs` $^ -o $@

psppmview: psppmview.o
	$(CC) `sdl-config --libs` $^ -o $@
