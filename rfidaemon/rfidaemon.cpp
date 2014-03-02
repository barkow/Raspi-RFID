#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <curl/curl.h>
#include "rfidreader.h"
#include <string>
using namespace std;

typedef void (*sighandler_t)(int);

#define SOURCENAME "TestSource"
#define LOGMESSAGE(message) printf(message)

static void shutdownProcess(int signr) {
  if (signr == SIGTERM) {
    LOGMESSAGE("Bye\n");
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


int sendPostRequest(char* source, string owner){
  CURL *curl;
  CURLcode res;
  struct curl_slist *header = NULL;
  #define JSONOBJ "{ \"source\" : \"%s\" , \"owner\" : \"%s\" }"
  char jsonObj[255];

  curl = curl_easy_init();
  curl_easy_setopt(curl, CURLOPT_URL, "http://localhost/rfidEventStorage.php");
  sprintf(jsonObj, JSONOBJ, source, owner.c_str());
  header = curl_slist_append(header, "Content-Type: application/json");
  curl_easy_setopt(curl, CURLOPT_POST, 1);
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonObj);
  res = curl_easy_perform(curl);
  curl_easy_cleanup(curl);
}

int main (int argc, char *argv[])
{
  unsigned int *rfidData;
  LOGMESSAGE("This is rfidaemon\n");

  rfidReaderClass *rfidReader = new rfidReaderClass();

  //Handler für das Beenden des Prozesses anmelden
  signal_add(SIGTERM, shutdownProcess);
  signal(SIGPIPE, SIG_IGN);

  //socketDesc = createSocket();
  while (1)
  {
    //Prüfen, ob ein gültiges Paket empfangen wurde
    string tag = rfidReader->getTag();
    if (tag != "") {
      sendPostRequest(SOURCENAME, tag);
    }
    sleep(200000);
  }
}
