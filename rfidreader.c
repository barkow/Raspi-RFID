#include <stdio.h>
#include <string.h>
#include "PIGPIO/pigpio.h"

/*
cc -o rfidreader rfidreader.c -L./PIGPIO -lpigpio -lrt -pthread
*/

#define EM4095_DEMOD 11 /* GPIO 11 as interrupt input */
#define TICKDURATION 100
#define BITLENGTH 100

#define ClearBuffer() bufferState = empty
static unsigned int dataBuffer[13];
static enum states {empty, header, data, stop, full} bufferState = empty;

void AddBitToBuffer(unsigned int bit) {
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
        bufferState = full;
      }
      break;
    case full:
      break;
    default:
      break;
  }
}

void pinChanged(int pin, int level, uint32_t tick) {
  static uint32_t lastRisingEdge = 0;
  uint32_t timeDelta;
  #define BITLENGTH_UNKNOWN 0
  #define BITLENGTH_1   1
  #define BITLENGTH_1_5 2
  #define BITLENGTH_2   3
  unsigned char bitLengthDetected = BITLENGTH_UNKNOWN;
  unsigned char isBitPositionMiddle = 0;
  unsigned char isBitPositionDetermined = 0;

  //Prüfen, ob Puffer bereit ist. Sonst Routine sofort verlassen
  if (bufferState == full) {
    return;
  }

  printf("Pin change detected\n");
  //Steigende Flanke erkennen
  if (level > 0) {
    printf("Rising edge detected\n");
    timeDelta = tick - lastRisingEdge;
    lastRisingEdge = tick;

    if ((timeDelta >= (0.75 * BITLENGTH)) && (timeDelta < (1.25 * BITLENGTH))) {
      bitLengthDetected = BITLENGTH_1;
    }
    else {
      if ((timeDelta >= (1.25 * BITLENGTH)) && (timeDelta < (1.75 * BITLENGTH))) {
        bitLengthDetected = BITLENGTH_1_5;
        //Wenn 1.5 Bitlängen zwischen zwei Flanken liegen, dann ändert sich die Bitposition
        isBitPositionMiddle = !isBitPositionMiddle;
      }
      else {
        if ((timeDelta >= (1.75 * BITLENGTH)) && (timeDelta < (2.25 * BITLENGTH))) {
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

void test(){
  AddBitToBuffer(1);
  AddBitToBuffer(1);
  AddBitToBuffer(1);
  AddBitToBuffer(1);
  AddBitToBuffer(1);
  AddBitToBuffer(1);
  AddBitToBuffer(1);
  AddBitToBuffer(1);
  AddBitToBuffer(1);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(1);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(1);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(1);
  AddBitToBuffer(1);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(1);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(1);
  AddBitToBuffer(0);
  AddBitToBuffer(1);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(1);
  AddBitToBuffer(1);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(1);
  AddBitToBuffer(1);
  AddBitToBuffer(1);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(1);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(1);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(1);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(1);
  AddBitToBuffer(0);
  AddBitToBuffer(1);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(1);
  AddBitToBuffer(0);
  AddBitToBuffer(1);
  AddBitToBuffer(1);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(1);
  AddBitToBuffer(1);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(1);
  AddBitToBuffer(1);
  AddBitToBuffer(0);
  AddBitToBuffer(1);
  AddBitToBuffer(0);
  AddBitToBuffer(0);
  AddBitToBuffer(0);

  printf("Buffer State: %i\n", bufferState);
  if (bufferState == full) {
    printf("Data  0: %i\n", dataBuffer[0]);
    printf("Data  1: %i\n", dataBuffer[1]);
    printf("Data  2: %i\n", dataBuffer[2]);
    printf("Data  3: %i\n", dataBuffer[3]);
    printf("Data  4: %i\n", dataBuffer[4]);
    printf("Data  5: %i\n", dataBuffer[5]);
    printf("Data  6: %i\n", dataBuffer[6]);
    printf("Data  7: %i\n", dataBuffer[7]);
    printf("Data  8: %i\n", dataBuffer[8]);
    printf("Data  9: %i\n", dataBuffer[9]);
    printf("Data 10: %i\n", dataBuffer[10]);
    printf("Data 11: %i\n", dataBuffer[11]);
    printf("Data 12: %i\n", dataBuffer[12]);
  }
}

int main (int argc, char *argv[])
{
  printf("This is rfidreader\n");

  test();

  return 0;

  if (gpioInitialise()<0) return 1;

  if (gpioSetAlertFunc(EM4095_DEMOD, pinChanged) != 0) {
   printf("Alert Function Registration failed\n");
  }
  gpioSetMode(EM4095_DEMOD, PI_INPUT);

  while (1)
  {
    //Prüfen, ob ein gültiges Paket empfangen wurde
    if (bufferState == full) {
      //Daten ausleses
      printf("ID detected: %i\n", dataBuffer[7] << 8 + dataBuffer[8] >> 4); 
      ClearBuffer();
    }
    gpioDelay(1000000);
  }
  
  gpioTerminate();
}
