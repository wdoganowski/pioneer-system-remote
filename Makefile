#
CC      = gcc
AR      = ar
RANLIB  = ranlib
SIZE    = size

CFLAGS  = -O3 -Wall

ALL     = gpiotest                                  

LL      = -L. -lpigpiod_if -lpthread -lrt

all:    $(ALL)

gpiotest: gpiotest.o       
	$(CC) -o gpiotest gpiotest.c $(LL)

clean:
	rm -f *.o *.i *.s *~ $(ALL)

# generated using gcc -MM *.c

gpiotest.o: gpiotest.c


