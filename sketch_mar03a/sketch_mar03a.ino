#include "DFRobot_BMI160.h"
#include <Adafruit_HMC5883_U.h>
#include <Wire.h>

DFRobot_BMI160 bmi160;
Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);

void setup() {
    Serial.begin(115200);
    Wire.begin();
    Wire.setClock(50000);

    // ✅ I2C 주소 확인 후 변경 필요 (0x68 또는 0x69)
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

void loop() {
    int16_t accelData[3];
    int16_t gyroData[3];

    // ✅ 센서 초기화 후 데이터 읽기 시도
    if (bmi160.getAccelData(accelData) != BMI160_OK) {
        Serial.println("Failed to read accelerometer data!");
    }
    if (bmi160.getGyroData(gyroData) != BMI160_OK) {
        Serial.println("Failed to read gyroscope data!");
    }

     // ✅ HMC5883L 데이터 읽기
    sensors_event_t event;
    mag.getEvent(&event);

    // ✅ 방위각(Yaw) 계산
    float heading = atan2(event.magnetic.y, event.magnetic.x);
    if (heading < 0) heading += 2 * PI;
    float headingDegrees = heading * 180 / PI;

    // ✅ 출력
    Serial.print("Accel X: "); Serial.print(accelData[0]);
    Serial.print(" Y: "); Serial.print(accelData[1]);
    Serial.print(" Z: "); Serial.println(accelData[2]);

    Serial.print("Gyro X: "); Serial.print(gyroData[0]);
    Serial.print(" Y: "); Serial.print(gyroData[1]);
    Serial.print(" Z: "); Serial.println(gyroData[2]);

    Serial.print("Mag X: "); Serial.print(event.magnetic.x);
    Serial.print(" Y: "); Serial.print(event.magnetic.y);
    Serial.print(" Z: "); Serial.print(event.magnetic.z);
    Serial.print("  Heading (Yaw): "); Serial.print(headingDegrees);
    Serial.println("°");

    Serial.println("-------------------------");
    
    delay(500);
}
