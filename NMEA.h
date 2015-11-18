/*
NMEA.h: C++ header for for NMEA parsing library

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

        int makeInt(int pos);

        float makeFloat(int pos);

        void makeTime(Time & t, int pos);

        void makeDate(Date & d, int pos);

        void makeCoordinate(Coordinate & c, int pos);

        void makeHeight(Height & h, int pos);

        int hemiSign(int pos);

        bool available(int pos);

        int safeInt(int pos);

        float safeFloat(int pos);

        NMEA_Message();

     public:

        char debug[100];

        void dump();

        NMEA_Message(char * msg);

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

        GPGGA_Message(char * msg);
};

class GPGLL_Message : public NMEA_Message {

    public:

        Coordinate latitude;
        Coordinate longitude;

        Time time;

        GPGLL_Message(char * msg);
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

        GPGSA_Message(char * msg);

};

class GPGSV_Message : public NMEA_Message {

    public:

        int nmsgs;
        int msgno;
        int nsats;

        int ninfo;

        SatelliteInView svs[4];

        GPGSV_Message(char * msg);
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

        GPRMC_Message(char * msg);

        static void serialize(char * msg, float latitude, float longitude);
};

class GPVTG_Message : public NMEA_Message {

    public:

        float trackMadeGoodTrue;
        float trackMadeGoodMagnetic;
        float speedKnots;
        float speedKPH;

        GPVTG_Message(char * msg);
};

class NMEA_Parser {

    private:

        char msg[200];
        char *pmsg;

        bool ismsg(const char * code);

    protected:

        NMEA_Parser();

        virtual void handleGPGGA(GPGGA_Message & gpgga);

        virtual void handleGPGLL(GPGLL_Message & gpgll);

        virtual void handleGPGSA(GPGSA_Message & gpgsa);

        virtual void handleGPGSV(GPGSV_Message & gpgsv);

        virtual void handleGPRMC(GPRMC_Message & gprmc);

        virtual void handleGPVTG(GPVTG_Message & gpvtg);

    public:

        void parse(char c);
};
