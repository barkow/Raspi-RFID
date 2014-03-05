#ifndef _USERINTERFACE_H_
#define _USERINTERFACE_H_

#include <time.h>

class userInterfaceClass{
    public:
        void handler();
        userInterfaceClass();
        enum states {ready, recognized, accepted, denied, failure};
        void setState(enum states newState);
    private:
        enum states interfaceState = ready;
        time_t lastChange;
};
#endif
