#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "Gukgung_Wifi";
const char* password = "love00007";
IPAddress local_IP(192, 168, 4, 4);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

WiFiUDP Udp;
IPAddress udpAddress;  // 유니티가 실행되는 PC의 IP 주소
const int udpPort = 12348;  // SW-420 센서 데이터 전송 포트
bool ipAcquired = false;

const int shockSensorPin = D1;  // SW-420 센서가 연결된 핀

void setup() {
  Serial.begin(115200);
  
  pinMode(shockSensorPin, INPUT);

  setWiFiConnect();

  Udp.begin(udpPort);
}

void loop() {
  if (!ipAcquired) 
    waitForPCIP();
  else
    sendSensorData();
    waitForCheckMessage();
}

void sendSensorData()
{
  int shockValue = digitalRead(shockSensorPin);

  if (shockValue == HIGH) {
    // 충격 감지 시 데이터 전송
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "Shock detected");
    Udp.beginPacket(udpAddress, udpPort);
    Udp.write(buffer);
    Udp.endPacket();

    // 디버깅을 위해 Serial 출력
    Serial.println("Shock detected!");

    delay(2500);  // 충격 감지 후 잠시 대기
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
