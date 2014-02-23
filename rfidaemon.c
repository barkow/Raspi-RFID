#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "PIGPIO/pigpio.h"


typedef void (*sighandler_t)(int);

#define PORT 6378 //Port auf dem der Socket horcht
#define LOGMESSAGE(message) printf(message)

static void shutdownProcess(int signr) {
  if (signr == SIGTERM) {
    LOGMESSAGE("Bye\n");
    rfidDeinit();
    exit(EXIT_SUCCESS);
  }
}

static sighandler_t signal_add (int sig_nr, sighandler_t signalhandler) {
  struct sigaction add_sig;
  if (sigaction (sig_nr, NULL, &add_sig) < 0){
    return SIG_ERR;
  }
  add_sig.sa_handler = signalhandler;
  sigaddset (&add_sig.sa_mask, sig_nr);
  add_sig.sa_flags = SA_RESTART;
  if (sigaction (sig_nr, &add_sig, NULL) < 0){
    return SIG_ERR;
  }
  return add_sig.sa_handler;
}

int createSocket(){
  int socketDesc;
  struct sockaddr_in server;

  socketDesc = socket(AF_INET, SOCK_STREAM, 0);
  if (socketDesc == -1){
    exit(EXIT_FAILURE);
  }

  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(PORT);

  if (bind(socketDesc, (struct sockaddr *)&server, sizeof(server)) < 0){
    exit(EXIT_FAILURE);
  }

  listen(socketDesc, 1);

  return socketDesc;
}

int waitForConnection(int socketDesc){
  int c, clientSock;
  struct sockaddr_in client;

  c = sizeof(struct sockaddr_in);
  clientSock = accept(socketDesc, (struct sockaddr *)&client, (socklen_t*)&c);
  if (clientSock < 0) {
    exit(EXIT_FAILURE);
  }
  return clientSock;
}

int main (int argc, char *argv[])
{
  int socketDesc;
  int clientSock = -1;
  unsigned int *rfidData; 
  LOGMESSAGE("This is rfidaemon\n");

  rfidInit();

  //Handler für das Beenden des Prozesses anmelden
  signal_add(SIGTERM, shutdownProcess);

  socketDesc = createSocket();

  while (1)
  {
    //Warten, bis Verbindung mit Socket aufgebaut wurde
    if (clientSock < 0){
      clientSock = waitForConnection(socketDesc);
    }

    //Prüfen, ob ein gültiges Paket empfangen wurde
    if (rfidCheck(rfidData) == 0) {
      //Daten ausleses
      if (write(clientSock, "Hallo", strlen("Hallo"))< 0){
        LOGMESSAGE("Socket disconnected\n");
        clientSock = -1;
      }
    }
  }
}
