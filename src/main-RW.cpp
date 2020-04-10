#define M5STACK_MPU6886

#include <Arduino.h>
#include <M5Stack.h>
#include <esp_now.h>
#include <WiFi.h>
#include <Ewma.h> // https://github.com/jonnieZG/EWMA
#include "common.h"
#include "ArduinoNvs.h"
#include "audio.h"

#define ROL_TOLERANC 30.0F
#define WING_TOLERANCE 1.0F

struct_message message;


bool do_sound = true;

void display_error(int e_num) {
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(0, 220);
    M5.Lcd.printf("E%d", e_num);
}

void set_pitch_diff_set(float val) {
    if (!NVS.setFloat("pitch", val)) display_error(1);
}

float get_pitch_diff_set() {
    float val = NVS.getFloat("pitch");
    if (val == 0) {
        val = 0.0F;
        set_pitch_diff_set(val);
    }
    return val;
}

float wing_diff = 0.0F;
float wing_diff_set = 0.0F;

Ewma filter(ALPHA);

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
    //Serial.println(WiFi.macAddress());
    memcpy(&message, incomingData, sizeof(message));

    float accX;
    float accY;
    float accZ;
    M5.IMU.getAccelData(&accX, &accY, &accZ);

    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(0, 50);
    M5.Lcd.printf(" % 01.3f   % 01.3f   % 01.3f", accX, accY, accZ);

    float yaw;
    float pitch;
    float roll;
    //M5.IMU.getAhrsData(&pitch, &roll, &yaw);
    yaw = D180 * std::atan(accZ / std::sqrt(accX * accX + accZ * accZ)) / M_PI;
    pitch = D180 * std::atan(accX / std::sqrt(accY * accY + accZ * accZ)) / M_PI;
    roll = D180 * std::atan(accY / std::sqrt(accX * accX + accZ * accZ)) / M_PI;
    float fp = filter.filter(pitch);
    if (isnan(fp)) display_error(2);
    if (isnan(message.filtered_pitch)) display_error(3);
    wing_diff = fp - message.filtered_pitch;
    M5.Lcd.setTextSize(8);
    M5.Lcd.setCursor(20, 100);
    float dif = wing_diff - wing_diff_set;
    M5.Lcd.printf("%+03.3f    ", dif);
    if (do_sound) {
        if (roll > -ROL_TOLERANC && roll < ROL_TOLERANC && message.roll > -ROL_TOLERANC &&
            message.roll < ROL_TOLERANC) {
            Say(dif, WING_TOLERANCE);
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
    M5.Lcd.setBrightness(50);

    NVS.begin();
    //M5.Speaker.setVolume(1);  // todo -- doesn't work!
    wing_diff_set = get_pitch_diff_set();
}

void loop() {
    M5.update();
    if (M5.BtnA.wasReleased()) {
        wing_diff_set = wing_diff;
        set_pitch_diff_set(wing_diff_set);
        Say();
    } else if (M5.BtnA.pressedFor(10000)) { // reset to 0.0 !
        set_pitch_diff_set(0.0F);
        Say();
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
