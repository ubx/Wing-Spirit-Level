#define M5STACK_200Q

#include <Arduino.h>
#include <M5Stack.h>
#include <esp_now.h>
#include <WiFi.h>
#include <Kalman.h>
#include "common.h"

// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
struct_message message;

uint32_t timer;
float dt;

Kalman kalmanY;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    M5.Lcd.setCursor(0, 150);
    M5.Lcd.print(status == ESP_NOW_SEND_SUCCESS ? "Sent OK" : "Sent Fail");
}

void setup() {
    Serial.begin(115200);

    M5.begin();
    M5.Power.begin();

    WiFi.mode(WIFI_STA);
    WiFi.setTxPower(WIFI_POWER_19_5dBm);

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    esp_now_register_send_cb(OnDataSent);

    // Register peer
    esp_now_peer_info_t peerInfo;
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    // Add peer
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add peer");
        return;
    }

    M5.IMU.Init();
    M5.IMU.setAccelFsr(M5.IMU.AFS_4G); // 200Q sensor, ±4g
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(GREEN, BLACK);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.setTextSize(4);
    M5.Lcd.print("Left Wing");
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(240, 220);
    M5.Lcd.print("OFF");
}

void loop() {
    dt = float((micros() - timer) / 1000000); //
    timer = micros();
    float accX;
    float accY;
    float accZ;
    M5.IMU.getAccelData(&accX, &accY, &accZ);

    float gyroX;
    float gyroY;
    float gyroZ;
    M5.IMU.getGyroData(&gyroX, &gyroY, &gyroZ);

    M5.Lcd.setCursor(0, 50);
    M5.Lcd.printf(" % 01.3f   % 01.3f   % 01.3f", accX, accY, accZ);

    //M5.IMU.getAhrsData(&message.pitch, &message.roll, &message.yaw);
    message.yaw = D180 * std::atan(accZ / std::sqrt(accX * accX + accZ * accZ)) / M_PI;
    message.pitch = D180 * std::atan(accX / std::sqrt(accY * accY + accZ * accZ)) / M_PI;
    message.roll = D180 * std::atan(accY / std::sqrt(accX * accX + accZ * accZ)) / M_PI;
    message.kalAngleY = kalmanY.getAngle(message.pitch, gyroY / 131.0F, dt);

    M5.Lcd.setCursor(0, 80);
    M5.Lcd.printf("yaw:   % 5.2f", message.yaw);
    M5.Lcd.setCursor(0, 100);
    M5.Lcd.printf("pitch: % 5.2f", message.pitch);
    M5.Lcd.setCursor(0, 120);
    M5.Lcd.printf("roll:  % 5.2f", message.roll);

    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &message, sizeof(message));
    if (result == ESP_OK) {
        Serial.println("Sent with success");
    } else {
        Serial.println("Error sending the data");
    }

    M5.update();
    if (M5.BtnC.pressedFor(5000)) {
        M5.Power.deepSleep();
    }

    delay(500);
}