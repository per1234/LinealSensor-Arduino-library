/**********************************************************************
 * LinealSensor library
 * Arduino library to control a lineal sensor usin an analog adapter
 * version 0.1 ALPHA 22/09/2015
 * Author: Jaime García  @peninquen
 * License: Apache License Version 2.0.
 *
 ********************************************************************
  */
//------------------------------------------------------------------------------
// Poner DEBUG a 1 para depuración.

#define DEBUG  0

//------------------------------------------------------------------------------
// Debug directives

#if DEBUG
#   define DEBUG_PRINT(...)    Serial.print(__VA_ARGS__)
#   define DEBUG_PRINTLN(...)  Serial.println(__VA_ARGS__)
#else
#   define DEBUG_PRINT(...)
#   define DEBUG_PRINTLN(...)
#endif


#include "LinealSensor.h"

/***************************************************************************/
/*constructor*/

LinealSensor::LinealSensor() {
}

/***************************************************************************/
/*Setup variables and initialize interrupts*/

void LinealSensor::begin(int sensorPin, unsigned short interval, calibration *cal, byte calAddress, unsigned short calNum) {
  _sensorPin = sensorPin;
  pinMode(_sensorPin, INPUT);  // analog input pin
  _flag = false;
  for (int i = 0; i < BUFFERSIZE; i++) _rawData[i] = 0;
  _index = 0;
  _cal =cal;
  for(int i = 0; i<calNum; i++){
    EEPROM.get(calAddress, _cal[i]);
    if (_cal[i].magicNumber != MAGIC_NUMBER) calibrate(calAddress, _cal[i]);
    calAddress += sizeof(calibration);
  }
//  _slope = (float)(_cal[1].reference - _cal[0].reference) / (_cal[1].rawData - _cal[0].rawData);
//  _offset = _cal[0].reference - _slope * _cal[0].rawData;
  _processTime = millis();       // start timer
  _interval = interval; //interval during loop function

  DEBUG_PRINT(F("Sensor Pin:")); DEBUG_PRINT(_sensorPin);
  DEBUG_PRINT(F("  EEPROM address:")); DEBUG_PRINTLN(calAddress);
}

/**************************************************************************/
/*calibrate sensor
  first calculate average value of 64 median values;
  second, use average to calculate standard deviation of the filtered and unfiltered readings*/
calibration &LinealSensor::calibrate(byte calAddress, calibration &cal) {
  unsigned short data;
  unsigned short StDev = 0;    //standard deviation of filtered data
  unsigned short rawStDev = 0; //standard deviation of unfiltered data
  unsigned long Dev = 0;       // squared deviation of filtered data
  unsigned long rawDev = 0;    // squared deviation of unfiltered data

  Serial.print("intro ref:");
  cal.reference = Serial.parseInt();
  for (int count = 64; count; --count) {
    while (!available()); // wait until data[] is full
    data += read();
  }
  cal.rawData = data >> 6;  //average value of 64 median values
  Serial.print("ADC value:");
  Serial.println(cal.rawData);

  for (int count = 64; count; --count) {
    for (int index = BUFFERSIZE; index; --index) {
      _rawData[index] = analogRead(_sensorPin);
      rawDev += (cal.rawData - _rawData[_index]) ^ 2;
    }
    for (int i = 0; i < BUFFERSIZE / 2; i++) {
      for (int j = i; j < BUFFERSIZE; j++) {
        if (_rawData[i] > _rawData[j]) {
          data = _rawData[i];
          _rawData[i] = _rawData[j];
          _rawData[j] = data;
        }
      }
    }
    Dev += (cal.rawData - _rawData[BUFFERSIZE / 2]) ^ 2;
  }
  StDev = sqrt(Dev >> 6);
  rawStDev = sqrt(rawDev / BUFFERSIZE / 64);
  Serial.print("filtered standard deviation:");
  Serial.println(StDev);
  Serial.print("un-filtered standard deviation:");
  Serial.println(rawStDev);
}

/***************************************************************************/
/*check interval and read data, interval must be greater than loop cycle*/
boolean LinealSensor::refreshData() {
  unsigned long nowTime = millis();
  if (nowTime - _processTime >= _interval) {
    _rawData[_index] = analogRead(_sensorPin);
    _processTime = nowTime;     //stamp process time
    if (++_index = BUFFERSIZE) {
      _flag = true;
      _index = 0;
    }
    return true;
  }
  return false;
}

/***************************************************************************/
/*read sensor value*/
// first, sort rawData array to get median value, rawData[BUFFERSIZE/2]
// next, make a linear transformation using calculated offset and slope
short LinealSensor::read() {
  unsigned short data;
  for (int i = 0; i < BUFFERSIZE / 2; i++) {
    for (int j = i; j < BUFFERSIZE; j++) {
      if (_rawData[i] > _rawData[j]) {
        data = _rawData[i];
        _rawData[i] = _rawData[j];
        _rawData[j] = data;
      }
    }
  }
  _flag = false;
  return (_offset + _slope * _rawData[BUFFERSIZE / 2]);
}

/***************************************************************************/
/* Is new data available */

boolean LinealSensor::available() {
  refreshData();
  return _flag;
}
