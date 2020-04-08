#define M5STACK_MPU6886

#include <Arduino.h>
#include <M5Stack.h>
#include <esp_now.h>
#include <WiFi.h>
#include "common.h"
#include "ArduinoNvs.h"
#include "audio.h"

#define ROL_TOLERANC 30.0F
#define WING_TOLERANCE 1.0F

struct_message message;

float yaw;
float pitch;
float roll;

bool do_sound = true;

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

float wing_diff = 0.0F;
float wing_diff_set = 0.0F;

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
    //Serial.println(WiFi.macAddress());
    memcpy(&message, incomingData, sizeof(message));

#ifdef DISPLAY_RAW
    float accX;
    float accY;
    float accZ;
    M5.IMU.getAccelData(&accX, &accY, &accZ);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(0, 50);
    M5.Lcd.printf(" %5.2f   %5.2f   %5.2f", accX, accY, accZ);
#endif
    M5.IMU.getAhrsData(&pitch, &roll, &yaw);
    wing_diff = pitch - message.pitch;
    M5.Lcd.setTextSize(8);
    M5.Lcd.setCursor(20, 100);
    M5.Lcd.printf("%5.3f    ", wing_diff_set - wing_diff);
    if (do_sound) {
        if (roll > -ROL_TOLERANC && roll < ROL_TOLERANC && message.roll > -ROL_TOLERANC &&
            message.roll < ROL_TOLERANC) {
            Say(wing_diff_set - wing_diff, WING_TOLERANCE);
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
    M5.IMU.setAccelFsr(M5.IMU.AFS_2G);  // MPU6886 sensor, ±2g
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(GREEN, BLACK);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.setTextSize(4);
    M5.Lcd.print("Right Wing");
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(50, 220);
    M5.Lcd.print("SET");
    M5.Lcd.setCursor(135, 220);
    M5.Lcd.print(do_sound ? "S-OFF" : "S-ON");
    M5.Lcd.setCursor(240, 220);
    M5.Lcd.print("OFF");

    NVS.begin();
    M5.Speaker.setVolume(1);  // todo -- doesn't work!
    wing_diff_set = get_pitch_diff_set();
}

void loop() {
    M5.update();
    if (M5.BtnA.wasReleased()) {
        wing_diff_set = wing_diff;
        set_pitch_diff_set(wing_diff_set);
        Say("Set!");
    } else if (M5.BtnB.wasReleased()) {
        do_sound = !do_sound;
        M5.Lcd.setTextSize(2);
        M5.Lcd.setCursor(135, 220);
        M5.Lcd.print(do_sound ? "S-OFF" : "S-ON ");
        Say();
    } else if (M5.BtnC.pressedFor(5000)) {
        M5.Power.deepSleep();
    }
}
