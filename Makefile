CC=gcc 
CFLAGS=-pthread -O2
LFLAGS=-L./PIGPIO 
LIBS=-lpigpio -lrt

all: rfidaemon

rfidaemon: rfidaemon.o rfidreader.o
	$(CC) -o $@ $^ $(CFLAGS) $(LFLAGS) $(LIBS)

rfidreader.o: rfidreader.c
rfidaemon.o: rfidaemon.c

clean:
	rm -f rfidaemon *.o
