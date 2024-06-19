#ifndef _Uduino_Wifi_H_
#define _Uduino_Wifi_H_
#include <Uduino.h>
#include <string.h>


#if defined(ESP8266)
    #define HARDWARE "ESP8266"
    #include <ESP8266WiFi.h>
#elif defined(ESP32)
    #define HARDWARE "ESP32"
    #include <WiFi.h>
#endif

#include <WiFiUdp.h>



  #ifdef  ARDUINO_ESP8266_WEMOS_D1MINI  // WeMos mini and D1 R2
    #define STATUS_LED 5
  #elif ARDUINO_ESP8266_ESP01           // Generic ESP's use for 01's
    #define STATUS_LED 0    
  #elif ARDUINO_ESP8266_NODEMCU               // Wio Link and NodeMCU 1.0 (also 0.9), use for ESP12
    #define STATUS_LED 14       
  #else          
    #define STATUS_LED 0       
  #endif 

class Uduino_Wifi : public Uduino
{
  public:
    Uduino_Wifi (const char* identity, const char* ssid, const char* password);
    Uduino_Wifi (const char* identity);
    static Uduino_Wifi * _instance;

    bool connectWifi(const char* ssid, const char* password);
    bool connectWifi(const char* ssid, const char* password, unsigned int portNumber);
    void useSerial(bool shouldUseSerial);
    void useSendBuffer(bool shouldUseBuffer);
    void setConnectionTries(unsigned int t);
    
    void setPort(unsigned int p);
    void setStaticIP(IPAddress ip, IPAddress gateway, IPAddress subnet);

    void update();
    static void printIdentity(char identity [] );

    size_t write(uint8_t);
    size_t write(const char *str) {
        if(str == NULL)
            return 0;
        return write((const uint8_t *) str, strlen(str));
    }
    size_t write(const uint8_t *buffer, size_t size);
    size_t write(const char *buffer, size_t size) {
        return write((const uint8_t *) buffer, size);}


    bool isWifiConnected();
    void delay(unsigned int duration);

  private:
    size_t sendWifiBuffer();
    size_t addToBuffer(const char *buffer, size_t size);
    size_t addToBuffer(uint8_t c);

    bool useWifi = false;
    WiFiUDP UDP_Receiver;
    unsigned int connectionTries = 35;  
    unsigned int port = 4222;  
    IPAddress remote = IPAddress(0,0,0,0);
    bool serialEnabled = false;

    //Static Settings
    IPAddress customIp;
    IPAddress customGateway;
    IPAddress customSubnet;


    // Buffer
    bool useBuffer = true;
    int sendBufferPosition = 0;  
    char sendBuffer[SEND_MAX_BUFFER];       // Buffer of stored characters while waiting for terminator character
};
#endif