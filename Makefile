CC=gcc
CFLAGS=-ggdb -Wall -Werror

.PHONY: all
all: leoimginfo
	

.PHONY: clean
clean:
	rm -f *.o leoimginfo

leogeo.o: Makefile leogeo.h leogeo.c
	$(CC) $(CFLAGS) -c leogeo.c

mfs.o: Makefile mfs.h mfs.c
	$(CC) $(CFLAGS) -c mfs.c
	
leoimginfo: Makefile leogeo.o mfs.o main.c
	$(CC) $(CFLAGS) main.c mfs.o leogeo.o -o leoimginfo