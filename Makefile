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

sha1.o: Makefile sha1.h sha1.c
	$(CC) $(CFLAGS) -c sha1.c
	
leoimginfo: Makefile leogeo.o mfs.o sha1.o main.c
	$(CC) $(CFLAGS) main.c mfs.o leogeo.o sha1.o -o leoimginfo
