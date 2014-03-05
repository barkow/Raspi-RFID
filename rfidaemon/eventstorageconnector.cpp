#include "eventstorageconnector.h"
#include <curl/curl.h>

using namespace std;

eventStorageConnectorClass::eventStorageConnectorClass(string source){
    this->source = source;
}

int eventStorageConnectorClass::addEvent(string owner){
  CURL *curl;
  CURLcode res;
  struct curl_slist *header = NULL;
  #define JSONOBJ "{ \"source\" : \"%s\" , \"owner\" : \"%s\" }"
  char jsonObj[255];

  curl = curl_easy_init();
  curl_easy_setopt(curl, CURLOPT_URL, "http://localhost/rfidEventStorage.php");
  sprintf(jsonObj, JSONOBJ, this->source.c_str(), owner.c_str());
  header = curl_slist_append(header, "Content-Type: application/json");
  curl_easy_setopt(curl, CURLOPT_POST, 1);
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonObj);
  //res = curl_easy_perform(curl);
  curl_easy_cleanup(curl);
}
