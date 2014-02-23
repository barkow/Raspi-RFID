#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>


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
    LOGMESSAGE("Error creating socket\n");
    exit(EXIT_FAILURE);
  }

  server.sin_family = AF_INET;
  //Nur lokale Verbindungen zulassen
  server.sin_addr.s_addr = inet_addr("127.0.0.1");
  server.sin_port = htons(PORT);

  if (bind(socketDesc, (struct sockaddr *)&server, sizeof(server)) < 0){
    LOGMESSAGE("Error binding socket\n");
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
    LOGMESSAGE("Error accepting socket connection\n");
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
  signal(SIGPIPE, SIG_IGN);

  socketDesc = createSocket();

  while (1)
  {
    //Warten, bis Verbindung mit Socket aufgebaut wurde
    if (clientSock < 0){
      clientSock = waitForConnection(socketDesc);
    }

    //Prüfen, ob ein gültiges Paket empfangen wurde
    if (rfidCheck(&rfidData) == 0) {
      char dataString[13*2+2] = "";
      int i;
      //Daten ausleses
      for (i=0; i<13; i++){
        char tmp[10];
        sprintf(tmp, "%02x", rfidData[i]);
        strcat(dataString, tmp);
      }
      strcat(dataString, "\n");      
      if (write(clientSock, dataString, strlen(dataString))< 0){
        LOGMESSAGE("Client disconnected\n");
        close(clientSock);
        clientSock = -1;
      }
    }
    sleep(200000);
  }
}
