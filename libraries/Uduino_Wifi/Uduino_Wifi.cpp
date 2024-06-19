#include "Uduino_Wifi.h"
 
Uduino_Wifi * Uduino_Wifi::_instance = NULL;

Uduino_Wifi::Uduino_Wifi(const char* identity, const char* ssid, const char* password): Uduino(identity) {
  _instance = this;
  delay(1000);
  Serial.begin(9600);
  connectWifi(ssid,password);
}

Uduino_Wifi::Uduino_Wifi(const char* identity): Uduino(identity) {
  _instance = this;
}

bool Uduino_Wifi::isWifiConnected() {
  return WiFi.status() == WL_CONNECTED;
}


void Uduino_Wifi::useSerial(bool shouldUseSerial) {
  serialEnabled = shouldUseSerial;
}

void Uduino_Wifi::useSendBuffer(bool shouldUseBuffer) {
  useBuffer = shouldUseBuffer;
}

void Uduino_Wifi::setConnectionTries(unsigned int t) {
  connectionTries = t;
}

void Uduino_Wifi::setPort(unsigned int p) {
  port = p;
}

void Uduino_Wifi::setStaticIP(IPAddress ip, IPAddress gateway, IPAddress subnet) {
  customIp = ip; 
  customGateway = gateway;  
  customSubnet = subnet;
}

bool Uduino_Wifi::connectWifi( const char* ssid, const char* password,unsigned int portNumber) {
  port = portNumber;
  return connectWifi(ssid, password);
}

bool Uduino_Wifi::connectWifi( const char* ssid, const char* password) {
  WiFi.persistent(false);
  WiFi.disconnect(true);
  delay(100);
  WiFi.mode(WIFI_OFF);
  delay(200);
  WiFi.mode(WIFI_STA);

  if(customIp != (uint32_t)0 && customGateway != (uint32_t)0) {
      WiFi.config(customIp, customGateway, customSubnet);
  }

  WiFi.begin(ssid, password);
  bool ledState = HIGH;
  unsigned int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < connectionTries)
  {
    Uduino::delay(500);
    Serial.print(".");
    digitalWrite(STATUS_LED, ledState);
    ledState = !ledState;
    tries ++;
  }
  if(tries >= connectionTries) {
    Serial.println("Can't connect to wifi!");
    useWifi = false;
    serialEnabled = true;
    return false;
  }
  Serial.print(" connected to ");
  Serial.println(ssid);
  UDP_Receiver.begin(port);
  Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), port);
  useWifi = true;
  addPrintFunction(printIdentity);
  return true;
}


size_t Uduino_Wifi::write(uint8_t c) {
 if(serialEnabled) {
     Serial.print(c);
  }
  if(useBuffer && init)
  {
      return addToBuffer(c);
  }
  else
  {
     if(remote == IPAddress(0,0,0,0) || (!useWifi)) {
       if(serialEnabled) return Serial.print(c);
       else return 0;
    } else {
      //int b = 
      UDP_Receiver.beginPacket(remote, port);
      size_t written = UDP_Receiver.write(c);
      //int ok = 
      UDP_Receiver.endPacket();
     // Uduino_Wifi::delay(5);
      return written;
    }
  }
 
}

 size_t Uduino_Wifi::write(const uint8_t *buffer, size_t size) {
  if(serialEnabled) {
      Serial.print((char*)buffer);
  }
  if(useBuffer && init)
  {
      return addToBuffer((char*)buffer,size);
  }
  else
  {
    if(remote == IPAddress(0,0,0,0) || (!useWifi) ) {
      return Serial.print((char*)buffer);
    } else {
      //int b = 
      UDP_Receiver.beginPacket(remote, port);

      #if defined(ESP8266)
       size_t written = UDP_Receiver.write((char*)buffer);
      #elif defined(ESP32)
       size_t written = UDP_Receiver.write(buffer, size);
      #endif


      //int ok = 
      UDP_Receiver.endPacket();
     // Uduino_Wifi::delay(10);
      return written;
    }
  }
 }

size_t Uduino_Wifi::addToBuffer(const char *buffer, size_t size) {
   // We add to existing buffer
  for(size_t i = 0; i < size;i++) {
    addToBuffer(buffer[i]);
  }
 // sendBuffer[sendBufferPosition] = buffer;
 // sendBufferPosition += size;
// sendWifiBuffer();
  return size;
}

size_t Uduino_Wifi::addToBuffer(uint8_t c) {
  // We add to existing buffer
  sendBuffer[sendBufferPosition] = c;
  sendBufferPosition ++;
  sendWifiBuffer();
  return 1;
}

size_t Uduino_Wifi::sendWifiBuffer() {

  if(sendBufferPosition > 1 && (sendBufferPosition >= SEND_MAX_BUFFER-1 ||
    (sendBuffer[sendBufferPosition -1] == '\n' && sendBuffer[sendBufferPosition -2] == '\r') // TODO : ça ça doit bugger
    ) )  {

   size_t written = 0;
    // Sending buffer 
   if(remote == IPAddress(0,0,0,0) || (!useWifi && serialEnabled) ) {
       written = Serial.print(sendBuffer);
    } else {
      UDP_Receiver.beginPacket(remote, port);

      #if defined(ESP8266)
      written = UDP_Receiver.write((char*)sendBuffer);
      #elif defined(ESP32)
        // TODO : strlen works only if the array is null terminated. Maybe I should enforce ?  
       written = UDP_Receiver.write((uint8_t*)sendBuffer, strlen(sendBuffer));
      #endif


      UDP_Receiver.endPacket();
    }

    // Clear buffer position and array
    sendBufferPosition = 0;
    for(int i = 0; i <SEND_MAX_BUFFER;i++) sendBuffer[i] = '\0';

    return written;

   } else {
    return 0;
  }
}


void Uduino_Wifi::printIdentity(char identity [] ) {
  Uduino_Wifi::_instance->println(identity);
}

void Uduino_Wifi::update() {
  Uduino::update(); // reads the serial
  if(!useWifi)
    return;

  int packetSize = UDP_Receiver.parsePacket();
  if (packetSize) {
    if(remote == IPAddress(0,0,0,0))
      remote = UDP_Receiver.remoteIP();
      if(serialEnabled) {
     //   serialEnabled = false;
       // Serial.println("Disabeling the use serial cause the board is already connected to the wifi.");
      }
    for(int i =0; i < packetSize; i++) {
      if(UDP_Receiver.available()) {

         char c = (char)UDP_Receiver.read();
         if(serialEnabled) Serial.print(c);
         Uduino::update(c);
       }
    }
  }
}

void Uduino_Wifi::delay(unsigned int duration) {
   unsigned long time_now = millis();
    while(millis() < time_now + duration) {
          Uduino_Wifi::update();
    }
}