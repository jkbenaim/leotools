CC=gcc
CFLAGS=-O0 -ggdb -Wall -Werror

.PHONY: all
all: leoimginfo ma2d1view
	

.PHONY: clean
clean:
	rm -f *.o leoimginfo ma2d1view

leogeo.o: Makefile leogeo.h leogeo.c
	$(CC) $(CFLAGS) -c leogeo.c

mfs.o: Makefile mfs.h mfs.c
	$(CC) $(CFLAGS) -c mfs.c

sha1.o: Makefile sha1.h sha1.c
	$(CC) $(CFLAGS) -c sha1.c
	
yay1.o: Makefile yay1.c yay1.h
	$(CC) $(CFLAGS) -c yay1.c
	
leoimginfo: Makefile leogeo.o mfs.o sha1.o main.c
	$(CC) $(CFLAGS) main.c mfs.o leogeo.o sha1.o -o leoimginfo

ma2d1view: Makefile yay1.o ma2d1view.c
	$(CC) $(CFLAGS) `sdl-config --libs --cflags` yay1.o ma2d1view.c -o ma2d1view