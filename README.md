# NMEA
NMEA parsing in C++ with Arduino example.

Currently supports the following NMEA messages:

  GPGGA
  GPGLL
  GPGSA
  GPGSV
  GPRMC
  GPVGT

For the Arduino example I used a HobbyKing Ublox Neo-7M GPS unit, which delivers NMEA messages over its TX line at 9600 baud.  The TX line is connected to the Arduino's RX1 port.
