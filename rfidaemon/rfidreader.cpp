#include "rfidreader.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "wiringPi.h"
//dummy implementations
#define wiringPiSetupGpio()
#define wiringPiISR(a,b,c)
#define micros() 10
#define delayMicroseconds(a) usleep(a)

#define EM4095_DEMOD 11 /* GPIO 11 as interrupt input */
#define BITLENGTH 256 //Bitlänge in us -> 1/125kHz*32
#define BIT1_LO   192 //0.75 * BITLENGTH
#define BIT1_HI   320 //1.25 * BITLENGTH
#define BIT1_5_LO 320 //1.25 * BITLENGTH
#define BIT1_5_HI 448 //1.75 * BITLENGTH
#define BIT2_LO   448 //1.75 * BITLENGTH
#define BIT2_HI   576 //2.25 * BITLENGTH

#define ClearBuffer() bufferState = empty
static unsigned int dataBuffer[13];
static enum states {empty, header, data, stop, full} bufferState = empty;

rfidReaderClass *rfidReaderClass::selfStatic = NULL;

void rfidReaderClass::AddBitToBuffer(unsigned int bit) {
  static unsigned int blockIndex = 0;
  static unsigned int innerBlockIndex = 0;
  static unsigned int headerIndex = 0;
  static unsigned int stopIndex = 0;

  switch (bufferState) {
    case empty:
      if (bit) {
        bufferState = header;
        headerIndex = 1;
      }
      break;
    case header:
      if (bit) {
        headerIndex++;
      }
      else {
        bufferState = empty;
      }
      if (headerIndex == 9) {
        bufferState = data;
        blockIndex = 0;
        innerBlockIndex = 0;
        dataBuffer[0] = 0;
      }
      break;
    case data:
      dataBuffer[blockIndex] = (dataBuffer[blockIndex] << 1) | bit;
      innerBlockIndex++;
      if (innerBlockIndex == 9) {
        //Wenn 9tes Bit nicht 0 ist, dann Fehler
        if (bit) {
          blockIndex = 0;
          bufferState = empty;
        }
        //9tes Bit wieder verwerfen
        dataBuffer[blockIndex] >>= 1;
        blockIndex++;
        innerBlockIndex = 0;
        dataBuffer[blockIndex] = 0;
      }
      if (blockIndex == 13) {
        stopIndex = 0;
        bufferState = stop;
      }
      break;
    case stop:
      if (!bit){
        stopIndex++;
      }
      else{
        bufferState = empty;
      }
      if (stopIndex == 2) {
        tagBuffer[2] = tagBuffer[1];
        tagBuffer[1] = tagBuffer[0];
        tagBuffer[0] = "";
        //Daten ausleses
        for (int i=0; i<13; i++){
          char tmp[10];
          sprintf(tmp, "%02x", dataBuffer[i]);
          tagBuffer[0] += tmp;
        }
        if ((tagBuffer[2] == tagBuffer[1]) && (tagBuffer[1] == tagBuffer[0])){
          bufferState = full;
        }
        else {
          bufferState = empty;
        }
      }
      break;
    case full:

      break;
    default:
      break;
  }
}

void rfidReaderClass::pinChanged() {
  int pin;
  int level;
  uint32_t tick;
  static uint32_t lastRisingEdge = 0;
  uint32_t timeDelta;
  #define BITLENGTH_UNKNOWN 0
  #define BITLENGTH_1   1
  #define BITLENGTH_1_5 2
  #define BITLENGTH_2   3
  unsigned char bitLengthDetected = BITLENGTH_UNKNOWN;
  static unsigned char isBitPositionMiddle = 0;
  static unsigned char isBitPositionDetermined = 0;

  //Prüfen, ob Puffer bereit ist. Sonst Routine sofort verlassen
  if (bufferState == full) {
    return;
  }

  //WiringPi übergibt den Level und den Zeitpunkt des Interrupts nicht als Funktionsparameter.
  //Da der Interrupt nur bei steigenden Flanken getriggrt wird, kann der Level hier als 1 angenommen werden
  //Der Zeitpunkt muss manuell ausgelesen werden
  level = 1;
  tick = micros();

  //Steigende Flanke erkennen
  if (level > 0) {
    timeDelta = tick - lastRisingEdge;
    lastRisingEdge = tick;
    if ((timeDelta >= BIT1_LO) && (timeDelta < BIT1_HI)) {
      bitLengthDetected = BITLENGTH_1;
    }
    else {
      if ((timeDelta >= BIT1_5_LO) && (timeDelta < BIT1_5_HI)) {
        bitLengthDetected = BITLENGTH_1_5;
        //Wenn 1.5 Bitlängen zwischen zwei Flanken liegen, dann ändert sich die Bitposition
        isBitPositionMiddle = !isBitPositionMiddle;
      }
      else {
        if ((timeDelta >= BIT2_LO) && (timeDelta < BIT2_HI)) {
          bitLengthDetected = BITLENGTH_2;
          //Wenn zwei Bitlängen zwischen den Flanken liegen, dass ist die aktuelle Position in der Mitte
          isBitPositionMiddle = 1;
          //In diesem Fall ist die Bitposition auf jeden Fall in der Mitte und damit eindeutig bestimmt
          isBitPositionDetermined = 1;
        }
        else {
          bitLengthDetected = BITLENGTH_UNKNOWN;
          //Wenn eine unbekannte Bitlänge erkannt wurde, muss die Bitposition zunächst erneut eindeutig bestimmt werden
          isBitPositionDetermined = 0;
          //printf("\n\n");
        }
      }
    }
    //Wenn die Bitposition eindeutig ist, kann das empfangene Signal dekodiert werden
    if(isBitPositionDetermined){
      switch(bitLengthDetected){
        case BITLENGTH_1:
          if (isBitPositionMiddle) {
            AddBitToBuffer(0);
          }
          else {
            AddBitToBuffer(1);
          }
          break;
        case BITLENGTH_1_5:
          if (isBitPositionMiddle) {
            AddBitToBuffer(1);
            AddBitToBuffer(0);
          }
          else {
            AddBitToBuffer(1);
          }
          break;
        case BITLENGTH_2:
            AddBitToBuffer(1);
            AddBitToBuffer(0);
          break;
      }
    }
    else { //isBitPositionDetermined is false
      ClearBuffer();
    }
  } //level>0
}

void rfidReaderClass::pinChangedStatic(){
    rfidReaderClass::selfStatic->pinChanged();
}

rfidReaderClass::rfidReaderClass() {
  rfidReaderClass::selfStatic = NULL;
  wiringPiSetupGpio();
  wiringPiISR(EM4095_DEMOD, INT_EDGE_RISING, rfidReaderClass::pinChangedStatic) ;
}

string rfidReaderClass::getTag(){
    if (bufferState == full){
        string retVal = tagBuffer[2];
        tagBuffer[0] = "";
        tagBuffer[1] = "";
        tagBuffer[2] = "";
        ClearBuffer();
        return retVal;
    }
    else {
        return "";
    }
}

void mysleep(uint32_t us){
  delayMicroseconds(us);
}
