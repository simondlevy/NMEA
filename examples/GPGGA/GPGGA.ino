/*
Simple NMEA parsing example for Arduino.  

Handles GPGGA messages by printing out latitude and longitude (degrees:minutes) and altitude (meters).

To run this code, copy the NMEA folder into your Arduino/libraries folder.

Copyright (C) Simon D. Levy 2015

This code is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as 
published by the Free Software Foundation, either version 3 of the 
License, or (at your option) any later version.
This code is distributed in the hope that it will be useful,     
but WITHOUT ANY WARRANTY without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU Lesser General Public License 
along with this code.  If not, see <http:#www.gnu.org/licenses/>.
*/
*/

#include <NMEA.h>

class MyParser : public NMEA_Parser {
  
  void handleGPGGA(GPGGA_Message & gpgga) 
  {
    this->dumpCoord(gpgga.latitude);
    Serial.print(" ");    
    this->dumpCoord(gpgga.longitude);
    Serial.print(" ");
    Height altitude = gpgga.altitude;
    Serial.print(altitude.value);
    Serial.print(altitude.units);
    Serial.println();
  }
  
  void dumpCoord(Coordinate coord) 
  {
    Serial.print(coord.sign == 1 ? "+" : "-");
    Serial.print(coord.degrees);
    Serial.print(":");
    Serial.print(coord.minutes);
  }

};

MyParser parser;

void setup() {
  
  Serial.begin(9600);
  Serial1.begin(9600);
}

void loop() {
  
  if (Serial1.available() > 0) {
    
        parser.parse(Serial1.read());
  }  
}
