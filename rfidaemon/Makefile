CC=g++
CFLAGS=-pthread -O2 -std=c++11
#GPIOLIB=-lwiringPi
LIBS=$(GPIOLIB) -lrt -lcurl

all: rfidaemon

rfidaemon: rfidaemon.o rfidreader.o userinterface.o eventstorageconnector.o
	$(CC) -o $@ $^ $(CFLAGS) $(LFLAGS) $(LIBS)

rfidaemon.o: rfidaemon.cpp rfidreader.h userinterface.h eventstorageconnector.h
	$(CC) $(CFLAGS) -c -o $@ $<

rfidreader.o: rfidreader.cpp rfidreader.h
	$(CC) $(CFLAGS) -c -o $@ $<

userinterface.o: userinterface.cpp userinterface.h
	$(CC) $(CFLAGS) -c -o $@ $<

eventstorageconnector.o: eventstorageconnector.cpp eventstorageconnector.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f rfidaemon *.o
