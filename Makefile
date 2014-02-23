CC=gcc 
CFLAGS=-pthread -O2
LFLAGS=-L./PIGPIO 
#GPIOLIB=-lpigpio
GPIOLIB=-lwiringPi
LIBS=$(GPIOLIB) -lrt

all: rfidaemon

rfidaemon: rfidaemon.o rfidreader.o
	$(CC) -o $@ $^ $(CFLAGS) $(LFLAGS) $(LIBS)

rfidreader.o: rfidreader.c
rfidaemon.o: rfidaemon.c

clean:
	rm -f rfidaemon *.o
