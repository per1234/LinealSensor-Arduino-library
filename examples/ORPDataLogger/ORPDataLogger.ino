/**********************************************************************
 * LinealSensor Logger example
 * An example to collect data from a lineal sensor conected to an analog adapter
 *  using the LinealSensor class
 *  ORP probe precision +-1 mV, we work with signed short numbers, value x1

 * version 0.1 ALPHA 22/09/2015
 * Author: Jaime García  @peninquen
 * License: Apache License Version 2.0.
 *
 **********************************************************************/

#include "LinealSensor.h"
#include <EEPROM.h>


#define N_ORPCAL 2
#define S_ORPCAL sizeof(calibration);
#define EM_ORPCAL_OFFSET 0
#define EM_ORPCAL_END (EM_ORPCAL_OFFSET + (N_ORPCAL * S_ORPCAL))
#define EM_END       (EM_ORPCAL_END + 1)

#define REFRESH_INTERVAL  1000   // refresh time, 1 second
#define WRITE_INTERVAL 5000      // values send to serial port, 5 seconds (5 * 1000)
#define ORP_PIN A0                //

calibration ORPcalibration[N_ORPCAL];
LinealSensor ORP; // instance to process data
//variables to process and send values
boolean firstData;
short ORPvalue;     // value x100
short ORPmax;
short ORPmin;
long ORPavg;
int counter = 0;

unsigned long previousMillis = 0;
unsigned long currentMillis = 0;


void setup() {
  Serial.begin(9600);
  delay(1000);
  ORPcalibration[0] = calibration(MAGIC_NUMBER, 0, 512);
  ORPcalibration[1] = calibration(MAGIC_NUMBER, 650, 1000); //include setup values to avoid calibration
    // LinealSensor::begin(
                      //int sensorPin, 
                      //unsigned short interval,
                      //calibration *cal, 
                      //byte calAddress, 
                      //unsigned short calNum
  
  ORP.begin(ORP_PIN, REFRESH_INTERVAL, ORPcalibration, EM_ORPCAL_OFFSET, N_ORPCAL);
  Serial.println("time(s), average ORP, max ORP, min ORP");

  firstData = false;
}

void loop() {
  if (ORP.available()) {
    ORPvalue = ORP.read();
    ORPavg += ORPvalue;
    counter++;
    if (!firstData) {
      if (ORPmax < ORPvalue) ORPmax = ORPvalue;
      if (ORPmin > ORPvalue) ORPmin = ORPvalue;
    }
    else {
      ORPmax = ORPvalue;
      ORPmin = ORPvalue;
      firstData = false;
    }
  }

  currentMillis = millis();
  if (currentMillis - previousMillis >= WRITE_INTERVAL) {
    previousMillis = currentMillis;
    firstData = true;
    ORPavg /= counter;
    counter = 0;

    Serial.print(currentMillis / 1000);
    Serial.print(",");
    Serial.print((float)ORPvalue / 100);
    Serial.print(",");
    Serial.print((float)ORPmax / 100);
    Serial.print(",");
    Serial.println((float)ORPmin / 100);

  }
}
