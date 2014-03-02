CC=g++ 
CFLAGS=-pthread -O2 -std=c++11
LFLAGS=-L./PIGPIO 
#GPIOLIB=-lpigpio
#GPIOLIB=-lwiringPi
LIBS=$(GPIOLIB) -lrt -lcurl

all: rfidaemon

rfidaemon: rfidaemon.o rfidreader.o
	$(CC) -o $@ $^ $(CFLAGS) $(LFLAGS) $(LIBS)

rfidreader.o: rfidreader.cpp rfidreader.h
	$(CC) $(CFLAGS) -c -o $@ $<
 
rfidaemon.o: rfidaemon.cpp rfidreader.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f rfidaemon *.o
