#define M5STACK_MPU6886

#include <Arduino.h>
#include <M5Stack.h>
#include <esp_now.h>
#include <WiFi.h>
#include <ArduinoQueue.h> // https://github.com/EinarArnason/ArduinoQueue
#include <ArduinoNvs.h>
#include <Ewma.h> // https://github.com/jonnieZG/EWMA
#include "common.h"
#include "audio.h"

#define ROL_TOLERANC 30.0F
#define WING_TOLERANCE 1.0F

typedef struct {
    struct_message other_msg;
    float accX;
    float accY;
    float accZ;
} queu_element;
const char *const OFFSET = "offset";
const char *const PITCH = "pitch";
ArduinoQueue<queu_element> queue(2);
bool do_sound = true;

void display_error(int e_num) {
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(0, 220);
    M5.Lcd.printf("E%d", e_num);
}

void set_pitch_diff_set(float val) {
    if (!NVS.setFloat(PITCH, val)) display_error(1);
}

void set_pitch_diff_offset(float val) {
    if (!NVS.setFloat(OFFSET, val)) display_error(1);
}

float get_pitch_diff_set() {
    float val = NVS.getFloat(PITCH);
    if (val == 0) {
        val = 0.0F;
        set_pitch_diff_set(val);
    }
    return val;
}

float get_pitch_diff_offset() {
    float val = NVS.getFloat(OFFSET);
    if (val == 0) {
        val = 0.0F;
        set_pitch_diff_offset(val);
    }
    return val;
}

float wing_diff = 0.0F;
float wing_diff_set = 0.0F;
float wing_diff_offset = 0.0F;

Ewma filter(ALPHA);
Ewma filterDiff(ALPHA_DIFF);

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
    //Serial.println(WiFi.macAddress());
    queu_element qe;
    memcpy(&qe.other_msg, incomingData, len);
    M5.IMU.getAccelData(&qe.accX, &qe.accY, &qe.accZ);
    queue.enqueue(qe);
}

void displayDate(const queu_element &qe) {
    if (qe.other_msg.powr_off) {
        M5.Power.powerOFF();
    }
    float pitch = calcPitch(qe.accX, qe.accY, qe.accZ);
    float roll = calcRoll(qe.accX, qe.accY, qe.accZ);
    wing_diff = filterDiff.filter(filter.filter(pitch) - qe.other_msg.filtered_pitch);
    float diff = wing_diff - wing_diff_set + wing_diff_offset;
#ifdef SERIAL_OUT
    Serial.printf("pitch= %01.3f  other pitch=%01.3f diff=%01.3f wing_diff=%01.3f wing_diff_set=%01.3f wing_diff_offset=%01.3f\n", pitch, qe.other_msg.filtered_pitch,
                  diff, wing_diff, wing_diff_set, wing_diff_offset);
#endif
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(0, 50);
    M5.Lcd.printf(" % 01.3f   % 01.3f   % 01.3f", qe.accX, qe.accY, qe.accZ);
    M5.Lcd.setTextSize(8);
    M5.Lcd.setCursor(20, 100);
    M5.Lcd.printf("%+03.3f    ", diff);
    if (do_sound) {
        if (roll > -ROL_TOLERANC && roll < ROL_TOLERANC && qe.other_msg.roll > -ROL_TOLERANC &&
            qe.other_msg.roll < ROL_TOLERANC) {
            Say(diff, WING_TOLERANCE);
        }
    }
}


void setup() {
#ifdef SERIAL_OUT
    // Initialize Serial Monitor
    Serial.begin(115200);
#endif
    M5.begin();
    M5.Power.begin();

    // Set device as a Wi-Fi Station
    WiFi.mode(WIFI_STA);

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

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
    M5.Lcd.setCursor(240, 220);
    M5.Lcd.print(do_sound ? "S-OFF" : "S-ON");
    M5.Lcd.setCursor(135, 220);
    M5.Lcd.print("CAL");
    M5.Lcd.setBrightness(50);

    NVS.begin();
    M5.Speaker.setVolume(8);
    wing_diff_set = get_pitch_diff_set();
    wing_diff_offset = get_pitch_diff_offset();

    esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
    while (!queue.isEmpty()) {
        displayDate(queue.dequeue());
    }
    M5.update();
    if (M5.BtnA.pressedFor(2000)) {
        wing_diff_set = wing_diff + wing_diff_offset;
        set_pitch_diff_set(wing_diff_set);
        Say();
    } else if (M5.BtnB.wasReleased()) {
        wing_diff_set - wing_diff_offset;
        wing_diff_offset = 0.0F - wing_diff;
        wing_diff_set + wing_diff_offset;
        set_pitch_diff_offset(wing_diff_offset);
        Say();
    } else if (M5.BtnC.wasReleased()) {
        do_sound = !do_sound;
        M5.Lcd.setTextSize(2);
        M5.Lcd.setCursor(240, 220);
        M5.Lcd.print(do_sound ? "S-OFF" : "S-ON ");
        Say();
    }
    delay(20);
}
