#
CC      = gcc
AR      = ar
RANLIB  = ranlib
SIZE    = size

CFLAGS  = -O3 -Wall

ALL     = pioneer install        

LL      = -L. -lpigpiod_if -lpthread -lrt

all:    $(ALL)

pioneer: pioneer.o       
	$(CC) -o pioneer pioneer.c $(LL)

clean:
	rm -f *.o *.i *.s *~ $(ALL)

install: pioneer
	sudo cp ./pioneer /usr/lib/cgi-bin/

# generated using gcc -MM *.c

pioneer.o: pioneer.c


