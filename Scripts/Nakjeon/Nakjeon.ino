#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "Gukgung_Wifi";
const char* password = "love00007";
IPAddress local_IP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

WiFiUDP Udp;
IPAddress udpAddress;  // 유니티가 실행되는 PC의 IP 주소
const int udpPort = 12345;
bool ipAcquired = false;

const int nakSensorPin = D1;

void setup() {
  Serial.begin(115200);
  
  pinMode(nakSensorPin, INPUT);

  setWiFiConnect();

  Udp.begin(udpPort);
}

void loop() {
  if (!ipAcquired) 
    waitForPCIP();
  else {
    sendSensorData();
    waitForCheckMessage();
  }
}

void sendSensorData()
{
  int nakValue = digitalRead(nakSensorPin);
  char buffer[256];
  if (nakValue == LOW) {
    // 센서 감지 시 데이터 전송
    snprintf(buffer, sizeof(buffer), "NakJeon");
    Udp.beginPacket(udpAddress, udpPort);
    Udp.write(buffer);
    Udp.endPacket();

    // 디버깅을 위해 Serial 출력
    Serial.println("NakJeon detected!");

    delay(500);  // 충격 감지 후 잠시 대기
  }
  else
  {
    snprintf(buffer, sizeof(buffer), "ReadyForValsi");
    Udp.beginPacket(udpAddress, udpPort);
    Udp.write(buffer);
    Udp.endPacket();

    // 디버깅을 위해 Serial 출력
    Serial.println("Ready");

    delay(500);
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
