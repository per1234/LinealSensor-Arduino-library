/**********************************************************************
 * LinealSensor library
 * Arduino library to control a lineal sensor using an analog adapter
 * the adapter signal must be conected to one of the analog pins
 * lineal sensor are pH probes, ORP probes. 
 * Including two point calibration procedure.
 *  
 *
 *
 *
 * version 0.1 ALPHA 22/09/2015
 * Author: Jaime García  @peninquen
 * License:  Apache License Version 2.0.
 *
/**********************************************************************/

#ifndef LinealSensor_h
#define LinealSensor_h

#include "Arduino.h"
#include <EEPROM.h>

#define BUFFERSIZE 10
#define MAGIC_NUMBER 1234 // byte number to check valid values in EEPROM
struct calibration {
  short magicNumber;
  short reference;      // pH reference value x100
  short rawData;        // ADC value

  calibration(short mN = 0, short ref = 0, short rData = 0 ) {
    magicNumber = mN;
    reference = ref;
    rawData = rData;
  }
};


class LinealSensor {
  public:
    //constructor
    LinealSensor();

    // Setup instance variables
    void begin(int sensorPin, unsigned short interval, calibration *cal, byte calAddress, unsigned short calNum);

    // calibrate and write data on EEPROM
    calibration &calibrate(byte calAddress, calibration &cal);

    // check interval and update data, interval must be greater than loop cycle
    boolean refreshData();

    // read _counter value in defined units
    short read();

    // is a new data available?
    boolean available();

  private:
    int _sensorPin;                       // Analog pin conected to pH sensor adapter
    boolean _flag;                        // true when data is available, false when data is readed
    unsigned long _processTime;           // last time process
    unsigned short _interval;             // time [miliseconds] between adquisition data
    unsigned short _rawData[BUFFERSIZE];  // raw data from ADC
    unsigned short _index;
    unsigned short _calNum;               // number of calibration references
    calibration *_cal;                    // Calibration reference
    float _offset;
    float _slope;
};

#endif
