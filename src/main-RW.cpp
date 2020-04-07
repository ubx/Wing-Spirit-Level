#define M5STACK_MPU6886

#include <Arduino.h>
#include <M5Stack.h>
#include <esp_now.h>
#include <WiFi.h>
#include <cmath>
#include "message_def.h"
#include <ESP8266SAM.h>  //https://github.com/earlephilhower/ESP8266SAM
#include <AudioOutputI2S.h>  //https://github.com/earlephilhower/ESP8266Audio
#include "ArduinoNvs.h"

#define D180 180.0

AudioOutputI2S *out = nullptr;
auto *sam = new ESP8266SAM;

struct_message message;

float accX = 0.0F;
float accY = 0.0F;
float accZ = 0.0F;


void set_pitch_diff_set(float val) {
    NVS.setFloat("pitch", val);
}

float get_pitch_diff_set() {
    auto val = NVS.getFloat("pitch");
    if (val == 0) {
        val = 0.0F;
        set_pitch_diff_set(val);
    }
    return val;
}

float pitch_diff = 0.0F;
float pitch_diff_set = 0.0F;

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
    //Serial.println(WiFi.macAddress());
    memcpy(&message, incomingData, sizeof(message));

    M5.IMU.getAccelData(&accX, &accY, &accZ);
    // Display our sensor data
#ifdef DISPLAY_RAW
    M5.Lcd.setCursor(0, 50);
    M5.Lcd.printf(" %5.2f   %5.2f   %5.2f", accX, accY, accZ);
#endif
    //float yaw = D180 * std::atan(accZ / std::sqrt(accX * accX + accZ * accZ)) / M_PI;
    float pitch = D180 * std::atan(accX / std::sqrt(accY * accY + accZ * accZ)) / M_PI;
    float roll = D180 * std::atan(accY / std::sqrt(accX * accX + accZ * accZ)) / M_PI;
    pitch_diff = pitch - message.pitch;
    float diff = pitch_diff - pitch_diff_set;
    M5.Lcd.setCursor(0, 80);
    M5.Lcd.printf("pitch_diff:     %5.2f", pitch_diff);
    M5.Lcd.setCursor(0, 100);
    M5.Lcd.printf("pitch_diff set: %5.2f", pitch_diff_set);
    if (roll > -30.0F && roll < 30.0F) {
        if (diff > 5.0F) {
            out->begin();
            sam->Say(out, "Lower!");
            out->stop();
        } else if (diff < -5.0F) {
            out->begin();
            sam->Say(out, "Higher!");
            out->stop();
        }
    }
}

void setup() {
    // Initialize Serial Monitor
    Serial.begin(115200);

    M5.begin();
    M5.Power.begin();

    // Set device as a Wi-Fi Station
    WiFi.mode(WIFI_STA);

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    // Once ESPNow is successfully Init, we will register for recv CB to
    // get recv packer info
    esp_now_register_recv_cb(OnDataRecv);

    M5.IMU.Init();
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(GREEN, BLACK);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.setTextSize(4);
    M5.Lcd.print("Right Wing");
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(50, 220);
    M5.Lcd.print("SET");

    out = new AudioOutputI2S(0, 1, 32);
    NVS.begin();
    pitch_diff_set = get_pitch_diff_set();
}

void loop() {
    M5.update();
    if (M5.BtnA.pressedFor(1000)) {
        pitch_diff_set = pitch_diff;
        set_pitch_diff_set(pitch_diff_set);
        out->begin();
        sam->Say(out, "Set!");
        out->stop();
    }
}
