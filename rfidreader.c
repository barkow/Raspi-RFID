#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#define USE_WIRINGPI
#undef USE_PIGPIO

#ifdef USE_WIRINGPI
#include "wiringPi.h"
#endif

#ifdef USE_PIGPIO
#include "PIGPIO/pigpio.h"
#endif

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

inline void AddBitToBuffer(unsigned int bit) {
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
        bufferState = full;
      }
      break;
    case full:
      break;
    default:
      break;
  }
}
#ifdef USE_WIRINGPI
void pinChanged() {
  int pin;
  int level;
  uint32_t tick;
#endif
#ifdef USE_PIGPIO
void pinChanged(int pin, int level, uint32_t tick) {
#endif
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
  
  #ifdef USE_WIRINGPI
  //WiringPi übergibt den Level und den Zeitpunkt des Interrupts nicht als Funktionsparameter. 
  //Da der Interrupt nur bei steigenden Flanken getriggrt wird, kann der Level hier als 1 angenommen werden
  //Der Zeitpunkt muss manuell ausgelesen werden
  level = 1;
  tick = micros();
  #endif

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

void rfidInit()
{
  #ifdef USE_PIGPIO
  gpioCfgClock(5, PI_CLOCK_PCM, PI_CLOCK_PLLD);

  if (gpioInitialise()<0){    
    exit(EXIT_FAILURE);
  }

  if (gpioSetAlertFunc(EM4095_DEMOD, pinChanged) != 0) {  
    exit(EXIT_FAILURE);
  }
  gpioSetMode(EM4095_DEMOD, PI_INPUT);
  #endif
  
  #ifdef USE_WIRINGPI
  wiringPiSetupGpio();
  wiringPiISR(EM4095_DEMOD, INT_EDGE_RISING,  pinChanged) ;
  #endif
}

void rfidDeinit(){
  #ifdef USE_PIGPIO
  gpioTerminate();
  #endif
}

int rfidCheck(unsigned int** dataBufferRef){
  int status = 0;
  if (bufferState == full) {
    //Daten auslesen    
    *dataBufferRef = dataBuffer;
    ClearBuffer();
  }
  else {
    status = -1;
  }
  
  return status;
}

void sleep(uint32_t us){
  #ifdef USE_PIGPIO
  gpioDelay(us);
  #endif
  
  #ifdef USE_WIRINGPI
  delayMicroseconds(us);
  #endif
}