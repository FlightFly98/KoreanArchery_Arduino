#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include "DFRobot_BMI160.h"
#include <Adafruit_HMC5883_U.h>

const char* ssid = "Gukgung_Wifi";
const char* password = "love00007";
IPAddress local_IP(192, 168, 4, 2);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

WiFiUDP Udp;
IPAddress udpAddress; // 유니티가 실행되는 PC의 IP 주소
const int udpPort = 12346;

bool ipAcquired = false;

DFRobot_BMI160 bmi160;
Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(67890);

void setup() {
  Serial.begin(115200);

  setWiFiAp();
  
  Wire.begin(D2, D1);
  
  initBmiHMC();
  
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

void setWiFiAp()
{
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(ssid, password);
  delay(10000); // AP 설정을 위한 시간 대기
  Serial.println("AP mode set");
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

void initBmiHMC() {
  int8_t initStatus = bmi160.I2cInit(0x69);
    if (initStatus != BMI160_OK) {
        Serial.print("BMI160 initialization failed! Error code: ");
        Serial.println(initStatus);
        while (1);
    }

    Serial.println("BMI160 initialized successfully!");

    delay(100);
    
    int16_t dummyData[3];
    if (bmi160.getAccelData(dummyData) != BMI160_OK || bmi160.getGyroData(dummyData) != BMI160_OK) {
        Serial.println("Sensor is not responding! Retrying...");
        delay(500);
        bmi160.I2cInit(0x69);
    }

    Serial.println("Reading accelerometer and gyroscope data...");

     if (!mag.begin()) {
        Serial.println("HMC5883L not detected! Check wiring.");
        while (1);
    }

    Serial.println("HMC5883L initialized successfully!");

    Serial.println("Reading sensor data...");
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


void sendSensorData() {
  int16_t acc_raw[3];
  int16_t gyro_raw[3];

  if (bmi160.getAccelData(acc_raw) != BMI160_OK) {
        Serial.println("Failed to read accelerometer data!");
    }
  if (bmi160.getGyroData(gyro_raw) != BMI160_OK) {
        Serial.println("Failed to read gyroscope data!");
    }

   // HMC5883L 데이터 읽기
   sensors_event_t event;
   mag.getEvent(&event);

   // 방위각(Yaw) 계산
   // float heading = atan2(event.magnetic.y, event.magnetic.x);
   // if (heading < 0) heading += 2 * PI;
   // float headingDegrees = heading * 180 / PI;

  char str[256];
  snprintf(str, sizeof(str), "Accel: %d, %d, %d, Gyro: %d, %d, %d, Mag: %d, %d",
  acc_raw[0], acc_raw[1], acc_raw[2],
  gyro_raw[0], gyro_raw[1], gyro_raw[2],
  (int16_t) event.magnetic.x, (int16_t) event.magnetic.y);
  
  Serial.println(str);

  Udp.beginPacket(udpAddress, udpPort);
  Udp.write(str);
  Udp.endPacket();

  delay(100);
}
