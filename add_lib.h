#include <SoftwareSerial.h> // Программный uart
#include <TinyGPS.h> // Расшифровка nmea
#include <ESP8266WiFi.h> //WiFi
#include <NTPClient.h>////time
#include <WiFiUdp.h>/////time
#include <Adafruit_NeoPixel.h> //Адресная лента
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif
