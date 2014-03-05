#ifndef _RFIDREADER_H_
#define _RFIDREADER_H_

#include <cstdint>
#include <string>
using namespace std;

void rfidDeinit();
int rfidCheck(unsigned int** dataBufferRef);
void mysleep(uint32_t us);

class rfidReaderClass{
    public:
    rfidReaderClass();
    string getTag();
    private:
    void pinChanged();
    static void pinChangedStatic();
    static rfidReaderClass *selfStatic;
    void AddBitToBuffer(unsigned int bit);
    string tagBuffer[3];
};

#endif
