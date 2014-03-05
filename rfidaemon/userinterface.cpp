#include "userinterface.h"

userInterfaceClass::userInterfaceClass(){
    this->lastChange = time(0);
}

void userInterfaceClass::setState(enum states newState){
    if(newState != this->interfaceState){
        switch(newState){
            case ready:

                break;
            case recognized:

                break;
            case accepted:

                break;
            case denied:

                break;
            case failure:

                break;
        }
        this->interfaceState = newState;
        this->lastChange = time(0);
    }
}

void userInterfaceClass::handler(){
    switch (this->interfaceState){
        case userInterfaceClass::accepted:
        case userInterfaceClass::denied:
            if(difftime(time(0), this->lastChange) > 10){
                this->setState(userInterfaceClass::ready);
            }
            break;
        default:
            break;
    }
}
