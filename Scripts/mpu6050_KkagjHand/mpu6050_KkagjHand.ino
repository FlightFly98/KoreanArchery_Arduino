#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>

const char* ssid = "WemosD1_ZoomHand";
const char* password = "love00007";
IPAddress local_IP(192, 168, 4, 2);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

WiFiUDP Udp;
IPAddress udpAddress;  // 유니티가 실행되는 PC의 IP 주소
const int udpPort = 12347;
bool ipAcquired = false;

void setup() {
  Serial.begin(115200);
  
  setWiFiConnect();
  
  Wire.begin(D2, D1);

  initializeMPU6050();

  Udp.begin(udpPort);

}

void loop() {
  if (!ipAcquired) {
    waitForPCIP();
  } else {
    sendSensorData();
    waitForCheckMessage();
  }
}

void setWiFiConnect()
{
  WiFi.begin(ssid, password);
  WiFi.config(local_IP, gateway, subnet);

  while (WiFi.status() != WL_CONNECTED) {
    delay(2000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
}

void initializeMPU6050() 
{
  // Power Management
  Wire.beginTransmission(0x68);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission();
  // Register 26
  for(uint8_t i = 2; i <= 7; i++)
  {
    Wire.beginTransmission(0x68);
    Wire.write(26);
    Wire.write(i << 3 | 0x01);
    Wire.endTransmission();
  }
  // Register 27
  Wire.beginTransmission(0x68);
  Wire.write(27);
  Wire.write(3 << 3);
  Wire.endTransmission();
  // Register 28
  Wire.beginTransmission(0x68);
  Wire.write(28);
  Wire.write(0);
  Wire.endTransmission();
}

void waitForPCIP() {
  int packetSize = Udp.parsePacket();
  if (packetSize) 
  {
    char packetBuffer[255];
    int len = Udp.read(packetBuffer, 255);
    if (len > 0) 
    {
      packetBuffer[len] = 0;
      Serial.println(packetBuffer);

      // PC IP 주소를 수신한 경우 ACK 응답 전송
      if (strcmp(packetBuffer, "Hello from PC") == 0) 
      { // 확인 메시지 추가
        Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
        Udp.write("ACK");
        Udp.endPacket();
        Serial.println("Sent ACK");

        udpAddress = Udp.remoteIP();
        Serial.print("Received packet from IP: ");
        Serial.println(udpAddress);
        ipAcquired = true;
        Serial.println("Set udpAddress");
      } 
    }
  }
}

void waitForCheckMessage() 
{
  int packetSize = Udp.parsePacket();
  if (packetSize) 
  {
    char packetBuffer[255];
    int len = Udp.read(packetBuffer, 255);
    if (len > 0) 
    {
      packetBuffer[len] = 0;
      Serial.println(packetBuffer);
      if(strcmp(packetBuffer, "Check connection") == 0)
      {
        Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
        Udp.write("ACK");
        Udp.endPacket();
        Serial.println("Sent ACK");
      }
    }
  }
}


void sendSensorData() 
{
  uint8_t i;
  static int16_t acc_raw[3]={0,}, gyro_raw[3]={0,};

  // Get Rssi
  int32_t rssi = WiFi.RSSI();
  
  // Get Accel
  Wire.beginTransmission(0x68);
  Wire.write(59);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 6);
  for(i = 0; i < 3; i++) acc_raw[i] = (Wire.read() << 8) | Wire.read();
  
  // Get Gyro
  Wire.beginTransmission(0x68);
  Wire.write(67);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 6);
  for(i = 0; i < 3; i++) gyro_raw[i] = (Wire.read() << 8) | Wire.read();

  char str[256];
  snprintf(str, sizeof(str), "RSSI: %d, Accel: %d, %d, %d, Gyro: %d, %d, %d",
  rssi,
  acc_raw[0], acc_raw[1], acc_raw[2],
  gyro_raw[0], gyro_raw[1], gyro_raw[2]);
  
  Serial.println(str);

  Udp.beginPacket(udpAddress, udpPort);
  Udp.write(str);
  Udp.endPacket();

  delay(100);
}
