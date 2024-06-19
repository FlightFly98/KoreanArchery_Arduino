// Uduino settings
#include <Uduino_Wifi.h>
Uduino_Wifi uduino("uduinoBoard"); // Declare and name your object

IPAddress ip(192, 168, 11, 11);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

void setup()
{
  Serial.begin(115200);

  uduino.setStaticIP(ip, gateway , subnet ); // IPAddress ip, IPAddress gateway, IPAddress subnet
  uduino.connectWifi("SSID", "password");
}

void loop()
{
  uduino.update();

  if (uduino.isConnected()) {
    uduino.println("This Uduino is connected with a static IP");
    uduino.delay(5000);
  }
}
