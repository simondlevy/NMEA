
/*
NMEA.cpp: C++ classes for for NMEA parsing library

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

#include "NMEA.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)|| defined(__AVR_ATmega2560__)
#include <Arduino.h>
static void print(char * s) {
    Serial.println(s);
}
#else
static void print(char * s) {
    printf("%s\n", s);
}
#endif

static char * nexttok() {

    return strtok(NULL, ",");
}

static int twodig(char * p, int k) {

    return 10 * (p[k]-'0') + (p[k+1]-'0');
}

NMEA_Parser::NMEA_Parser() {

    this->pmsg = this->msg;
}

bool NMEA_Parser::ismsg(const char * code) {

    return strncmp(code, this->msg, 5) == 0;
}

bool NMEA_Message::available(int pos) {

    return this->parts[pos][0] != 0;
}

int NMEA_Message::safeInt(int pos) {

    return available(pos) ? makeInt(pos) : -1;
}

float NMEA_Message::safeFloat(int pos) {

    return available(pos) ? makeFloat(pos) : -1;
}

int NMEA_Message::hemiSign(int pos) {

    char hemi = this->parts[pos][0];

    return (hemi == 'S' || hemi == 'W') ? -1 : +1;
}

int NMEA_Message::makeInt(int pos) {

    return atoi(this->parts[pos]);
}

float NMEA_Message::makeFloat(int pos) {

    return atof(this->parts[pos]);
}

void NMEA_Message::makeTime(Time & t, int pos) {

    char * p = this->parts[pos];

    t.hours   = twodig(p, 0);
    t.minutes = twodig(p, 2); 
    t.seconds = twodig(p, 4);
}

void NMEA_Message::makeDate(Date & d, int pos) {

    char * p = this->parts[pos];

    d.day   = twodig(p, 0);
    d.month = twodig(p, 2); 
    d.year  = twodig(p, 4);
}

void NMEA_Message::makeCoordinate(Coordinate & c, int pos) {

    char tmp[20];
    strcpy(tmp, this->parts[pos]);

    char * p = strchr(tmp, '.');
    *p = 0;

    ++p;

    int degmin = atoi(tmp);

    c.sign = hemiSign(pos+1);
    c.degrees = degmin / 100;
    c.minutes = degmin % 100 + atol(p) / pow(10, strlen(p));
}

void NMEA_Message::makeHeight(Height & h, int pos) {

    h.value = makeFloat(pos);

    h.units = this->parts[pos+1][0];
}


void NMEA_Parser::parse(char c) {

    // NMEA messages start with a dollar sign
    if (c == '$') {

        // null-terminate our copy of the message
        *(--this->pmsg) = 0;

        // only allow messages starting with GP*** (ignore startup noise)
        if (!strncmp(this->msg, "GP", 2)) {

            char * p = this->msg;

            // compute checksum from message contents, terminated by asterisk
            char sum = 0;
            while (*p && (*p != '*')) {
                sum ^= *p;
                p++;
            }

            // if checksum matches checksum after asterisk, process the message
            int chk;
            sscanf(++p, "%x", &chk);
            if (chk == sum) {
                if      (ismsg("GPGGA")) {
                    GPGGA_Message gpgga(this->msg);
                    handleGPGGA(gpgga);
                }
                else if (ismsg("GPGLL")) {
                    GPGLL_Message gpgll(this->msg);
                    handleGPGLL(gpgll);
                }
                else if (ismsg("GPGSA")) {
                    GPGSA_Message gpgsa(this->msg);
                    handleGPGSA(gpgsa);
                } 
                else if (ismsg("GPGSV")) {
                    GPGSV_Message gpgsv(this->msg);
                    handleGPGSV(gpgsv);
                } 
                else if (ismsg("GPRMC")) {
                    GPRMC_Message gprmc(this->msg);
                    handleGPRMC(gprmc);
                } 
                else if (ismsg("GPVTG")) {
                    GPVTG_Message gpvtg(this->msg);
                    handleGPVTG(gpvtg);
                } 
              }
        }

        this->pmsg = this->msg;
    }

    // for characters other than $, we store them as the current message    
    else {

        *(this->pmsg) = c;
        this->pmsg++;
    } 
}

static int num (char * s, int beg) {

    return 10 * (s[beg]-'0') + (s[beg+1]-'0');
}

NMEA_Message::NMEA_Message(char * msg) {

    this->nparts = 0; 
    int j = 0;
    char * p = msg;
    
    while (*p) {

        if (*p == ',') {

            this->parts[this->nparts][j] = 0;
            j = 0;
            this->nparts++;
        }
        else {

            this->parts[this->nparts][j] = *p;
            j++;
        }

        p++;
    }

    strcpy(this->debug, msg);
}

NMEA_Message::NMEA_Message() {
}

void NMEA_Message::dump() {

    for (int k=0; k<this->nparts; ++k) {

        print(this->parts[k]);
    }
    print((char *)"\n");
}

GPGGA_Message::GPGGA_Message(char * msg) : NMEA_Message(msg) {

    makeTime(this->time, 1);

    makeCoordinate(this->latitude, 2);
    makeCoordinate(this->longitude, 4);

    this->fixQuality = this->parts[6][0] - '0';

    this->numSatellites = makeInt(7);

    this->hdop = makeFloat(8);

    makeHeight(this->altitude, 9);

    makeHeight(this->geoid, 11);
}

GPGLL_Message::GPGLL_Message(char * msg) : NMEA_Message(msg) {

    makeCoordinate(this->latitude, 1);
    makeCoordinate(this->longitude, 3);

    makeTime(this->time, 5);
}

GPGSA_Message::GPGSA_Message(char * msg) : NMEA_Message(msg) {

    this->mode = this->parts[1][0];
    this->fixType = makeInt(2);

    this->nsats = 0;
    for (int k=3; k<=14; ++k) {
        this->satids[this->nsats++] = safeInt(k);
    }

    this->pdop = makeFloat(15);
    this->hdop = makeFloat(16);
    this->vdop = makeFloat(17);
} 



GPGSV_Message::GPGSV_Message(char * msg) : NMEA_Message(msg) {

    this->nmsgs     = makeInt(1);
    this->msgno     = makeInt(2);
    this->nsats     = makeInt(3);

    this->ninfo = (this->nparts - 3) / 4;

    for (int j=0; j<this->ninfo; ++j) {

        int k = (j+1) * 4;

        this->svs[j].prn       = makeInt(k);
        this->svs[j].elevation = makeInt(k+1);
        this->svs[j].azimuth   = makeInt(k+2);
        this->svs[j].snr       = safeInt(k+3);
    }
 }

GPRMC_Message::GPRMC_Message(char * msg) : NMEA_Message(msg) {

    makeTime(this->time, 1);

    this->warning = this->parts[2][0];

    makeCoordinate(this->latitude, 3);
    makeCoordinate(this->longitude, 5);

    this->groundspeedKnots = makeFloat(7);

    this->trackAngle = safeFloat(8);

    makeDate(this->date, 9);

    // XXX untested
    if (this->parts[10][0]) {

        this->magvar = hemiSign(11) * makeFloat(10);
    }
}

static char coord2str(float coord, char * str, const char * fmt) {

    coord = abs(coord);
    int intdeg = coord;
    float minutes = (coord-intdeg)*60;
    int intmin = minutes;
    int fracmin = 1000 * (minutes - intmin);
    sprintf(str, fmt, intdeg, intmin, fracmin);
}

GPVTG_Message::GPVTG_Message(char * msg) : NMEA_Message(msg) {

        this->trackMadeGoodTrue = safeFloat(1);
        this->trackMadeGoodMagnetic = safeFloat(3);
        this->speedKnots = safeFloat(5);
        this->speedKPH = safeFloat(7);
}

static void make_nmea(char * in, char *out) {

    char chk = 0;

    for (char * p = in; *p; p++)
        chk ^= *p;

    sprintf(out, "$%s*%02X\r", in, chk);
}

void GPRMC_Message::serialize(char * msg, float latitude, float longitude, float speed) {

    char latstr[20];
    coord2str(latitude, latstr, "%d%02d.%04d");

    char lonstr[20];
    coord2str(longitude, lonstr, "%03d%02d.%04d");

    char speedstr[10];
    sprintf(speedstr, "%03d.%d", (int)speed, (int)(10*(speed-(int)speed)));

    char tmp[200];
    sprintf(tmp, "GPRMC,170954,A,%s,%c,%s,%c,%s,0,161115,,,A", 
            latstr, latitude>0?'N':'S', lonstr, longitude>0?'E':'W', speedstr);
    make_nmea(tmp, msg);
}

// Overridden by implementation ---------------------------

void NMEA_Parser:: handleGPGGA(GPGGA_Message & gpgga) {}

void NMEA_Parser:: handleGPGLL(GPGLL_Message & gpgll) {}

void NMEA_Parser:: handleGPGSA(GPGSA_Message & gpgsa) {}

void NMEA_Parser:: handleGPGSV(GPGSV_Message & gpgsv) {}

void NMEA_Parser:: handleGPRMC(GPRMC_Message & gprmc) {}

void NMEA_Parser:: handleGPVTG(GPVTG_Message & gpvtg) {}
