#ifndef _EVENTSTORAGECONNECTOR_H_
#define _EVENTSTORAGECONNECTOR_H_

#include <string>

class eventStorageConnectorClass{
    public:
        eventStorageConnectorClass(std::string source);
        int addEvent(std::string owner);
    private:
        std::string source;
};
#endif
