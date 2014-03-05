#include "rfidreader.h"
#include "userinterface.h"
#include "eventstorageconnector.h"

#include <iostream>
#include <signal.h>
#include <stdlib.h>

#include <string>

using namespace std;

typedef void (*sighandler_t)(int);

#define SOURCENAME "TestSource"
#define LOGMESSAGE(message) cout << (message) << endl

bool exitLoop = false;

static void shutdownProcess(int signr) {
  exitLoop = true;
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

int main (int argc, char *argv[])
{
  unsigned int *rfidData;
  LOGMESSAGE("This is rfidaemon");

  rfidReaderClass *rfidReader = new rfidReaderClass();
  eventStorageConnectorClass *eventStorageConnector = new eventStorageConnectorClass(SOURCENAME);
  userInterfaceClass *userInterface = new userInterfaceClass();

  //Handler für das Beenden des Prozesses anmelden
  signal_add(SIGTERM, shutdownProcess);
  signal_add(SIGINT, shutdownProcess);
  signal(SIGPIPE, SIG_IGN);

  while (!exitLoop)
  {
    //Prüfen, ob ein gültiges Paket empfangen wurde
    string tag = rfidReader->getTag();
    if (tag != "") {
      userInterface->setState(userInterfaceClass::recognized);
      eventStorageConnector->addEvent(tag);
      userInterface->setState(userInterfaceClass::accepted);
    }
    mysleep(200000);
    userInterface->handler();
  }
  delete rfidReader;
  delete eventStorageConnector;
  delete userInterface;
  LOGMESSAGE("Bye\n");
  exit(EXIT_SUCCESS);
}
