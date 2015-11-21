/**
   NMEA.h: C++ header for for NMEA parsing/serializing library

   For NMEA protocol see http://aprs.gids.nl/nmea/ 

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

class Time {

    public:

        int hours;
        int minutes;
        int seconds;
};

class Date {

    public:

        int day;
        int month;
        int year;
};

class Coordinate {

    public:

        char  sign;
        int   degrees;
        float minutes;
};

class Height {

    public:

        float value;
        char units;
};

class SatelliteInView {

    public:

        int prn;
        int elevation;
        int azimuth;
        int snr;
};

class NMEA_Message {

    protected:

        char parts[20][30];
        int nparts;

        int makeInt(int pos) {
            return atoi(this->parts[pos]);
        }

        float makeFloat(int pos) {
            return atof(this->parts[pos]);
        }

        void makeTime(Time & t, int pos) {
            char * p = this->parts[pos];

            t.hours   = twodig(p, 0);
            t.minutes = twodig(p, 2); 
            t.seconds = twodig(p, 4);
        }

        void makeDate(Date & d, int pos) {

            char * p = this->parts[pos];

            d.day   = twodig(p, 0);
            d.month = twodig(p, 2); 
            d.year  = twodig(p, 4);
        }

        void makeCoordinate(Coordinate & c, int pos) {
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

        void makeHeight(Height & h, int pos) {
            h.value = makeFloat(pos);
            h.units = this->parts[pos+1][0];
        }


        int hemiSign(int pos) {
            char hemi = this->parts[pos][0];
            return (hemi == 'S' || hemi == 'W') ? -1 : +1;
        }

        bool available(int pos) {
            return this->parts[pos][0] != 0;
        }

        int safeInt(int pos) {
            return available(pos) ? makeInt(pos) : -1;
        }

        float safeFloat(int pos) {
            return available(pos) ? makeFloat(pos) : -1;
        }

        static char coord2str(float coord, char * str, const char * fmt) {

            coord = abs(coord);
            int intdeg = coord;
            float minutes = (coord-intdeg)*60;
            int intmin = minutes;
            int fracmin = 1000 * (minutes - intmin);
            sprintf(str, fmt, intdeg, intmin, fracmin);
        }

        NMEA_Message(char * msg) {

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

        NMEA_Message() {
        }

        static void make_nmea(char * in, char *out) {

            char chk = 0;

            for (char * p = in; *p; p++)
                chk ^= *p;

            sprintf(out, "$%s*%02X\r", in, chk);
        }

        static void float2str(float f, char * s) {

            sprintf(s, "%03d.%d", (int)f, (int)(10*(f-(int)f)));
        }

    public:

        char debug[100];

        void dump() {

            for (int k=0; k<this->nparts; ++k) {

                print(this->parts[k]);
            }
            print((char *)"\n");
        }

    private:

       static int twodig(char * p, int k) {

            return 10 * (p[k]-'0') + (p[k+1]-'0');
        }

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

};


class GPGGA_Message : public NMEA_Message {

    public:

        Time time;
        Coordinate latitude;
        Coordinate longitude;
        int fixQuality;
        int numSatellites;
        float hdop;
        Height altitude;
        Height geoid;

        GPGGA_Message(char * msg) : NMEA_Message(msg) {

            makeTime(this->time, 1);

            makeCoordinate(this->latitude, 2);
            makeCoordinate(this->longitude, 4);

            this->fixQuality = this->parts[6][0] - '0';

            this->numSatellites = makeInt(7);

            this->hdop = makeFloat(8);

            makeHeight(this->altitude, 9);

            makeHeight(this->geoid, 11);
        }

};

class GPGLL_Message : public NMEA_Message {

    public:

        Coordinate latitude;
        Coordinate longitude;

        Time time;
        GPGLL_Message(char * msg) : NMEA_Message(msg) {

            makeCoordinate(this->latitude, 1);
            makeCoordinate(this->longitude, 3);

            makeTime(this->time, 5);
        }
};

class GPGSA_Message : public NMEA_Message {

    public:

        char mode;
        int fixType;

        int satids[20];
        int nsats;

        float pdop;
        float hdop;
        float vdop;

        GPGSA_Message(char * msg) : NMEA_Message(msg) {

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
};

class GPGSV_Message : public NMEA_Message {

    public:

        int nmsgs;
        int msgno;
        int nsats;

        int ninfo;

        SatelliteInView svs[4];

        GPGSV_Message(char * msg) : NMEA_Message(msg) {

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
};


class GPRMC_Message : public NMEA_Message {

    public:

        Time time;
        char warning;
        Coordinate latitude;
        Coordinate longitude;
        float groundspeedKnots;
        float trackAngle;
        Date date;
        float magvar;

        GPRMC_Message(char * msg) : NMEA_Message(msg) {

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

        static void serialize(char * msg, 
                unsigned long time, 
                float latitude, 
                float longitude, 
                float speed, 
                float course, 
                unsigned long date,
                float magvar) {

            char latstr[20];
            coord2str(latitude, latstr, "%d%02d.%d");

            char lonstr[20];
            coord2str(longitude, lonstr, "%03d%02d.%d");

            char speedstr[10];
            float2str(speed, speedstr);

            char coursestr[10];
            float2str(course, coursestr);

            char magvarstr[10];
            float2str(abs(magvar), magvarstr);


            char tmp[200];
            sprintf(tmp, "GPRMC,%06d,A,%s,%c,%s,%c,%s,%s,%ld,%s,%c", 
                    (int)time, 
                    latstr, 
                    latitude>0?'N':'S', 
                    lonstr, longitude>0?'E':'W', 
                    speedstr, 
                    coursestr, 
                    date,
                    magvarstr,magvar>0?'E':'W');
            make_nmea(tmp, msg);
        }
};


class GPVTG_Message : public NMEA_Message {

    public:

        float trackMadeGoodTrue;
        float trackMadeGoodMagnetic;
        float speedKnots;
        float speedKPH;

        GPVTG_Message(char * msg) : NMEA_Message(msg) {

            this->trackMadeGoodTrue = safeFloat(1);
            this->trackMadeGoodMagnetic = safeFloat(3);
            this->speedKnots = safeFloat(5);
            this->speedKPH = safeFloat(7);
        }
};

class NMEA_Parser {

    private:

        char msg[200];
        char *pmsg;

        bool ismsg(const char * code) {
            return strncmp(code, this->msg, 5) == 0;
        }

    protected:

        NMEA_Parser() {
            this->pmsg = this->msg;
        }

        virtual void handleGPGGA(GPGGA_Message & gpgga) { }

        virtual void handleGPGLL(GPGLL_Message & gpgll) { }

        virtual void handleGPGSA(GPGSA_Message & gpgsa) { }

        virtual void handleGPGSV(GPGSV_Message & gpgsv) { }

        virtual void handleGPRMC(GPRMC_Message & gprmc) { }

        virtual void handleGPVTG(GPVTG_Message & gpvtg) { }

    public:

        void parse(char c) {

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
};
