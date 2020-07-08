#define M5STACK_MPU6886
#define BAT_TEST

#include <Arduino.h>
#include <M5Stack.h>
#include <Kalman.h>
#include <Ewma.h> // https://github.com/jonnieZG/EWMA

#define D180 180.0


float accX = 0.0F;
float accY = 0.0F;
float accZ = 0.0F;

float gyroX = 0.0F;
float gyroY = 0.0F;
float gyroZ = 0.0F;

uint32_t timer;
double dt;

Kalman kalmanX;
Ewma filter(0.5); // 0.0 .. 1.0

#ifdef BAT_TEST

int8_t getBatteryLevel() {
    Wire.beginTransmission(0x75);
    Wire.write(0x78);
    if (Wire.endTransmission(false) == 0
        && Wire.requestFrom(0x75, 1)) {
        switch (Wire.read() & 0xF0) {
            case 0xE0:
                return 25;
            case 0xC0:
                return 50;
            case 0x80:
                return 75;
            case 0x00:
                return 100;
            default:
                return 0;
        }
    }
    return -1;
}

#endif

void setup() {
    Serial.begin(115200);

    M5.begin();
    M5.Power.begin();
#ifndef BAT_TEST
    M5.IMU.Init();
    M5.IMU.setAccelFsr(M5.IMU.AFS_2G);  // MPU6886 sensor, ±2g

    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(GREEN, BLACK);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.setTextSize(4);
    M5.Lcd.print("Test MPU6886");
    M5.Lcd.setTextSize(2);
#endif
}

void loop() {


#ifdef BAT_TEST
    M5.Lcd.clear();
    M5.Lcd.setCursor(40, 20);
    M5.Lcd.setTextColor(GREEN);
    M5.Lcd.setTextSize(2);
    M5.Lcd.print("Battery Level: ");
    M5.Lcd.print(getBatteryLevel());
    M5.Lcd.print("%");
    delay(10000);
#else
    dt = (double) (micros() - timer) / 1000000; // Calculate delta time
    timer = micros();

    M5.IMU.getAccelData(&accX, &accY, &accZ);
    M5.IMU.getGyroData(&gyroX, &gyroY, &gyroZ);


    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(0, 50);
    M5.Lcd.printf(" %5.3f   %5.3f   %5.3f", accX, accY, accZ);

    float yaw;
    float pitch;
    float roll;
    yaw = D180 * std::atan(accZ / std::sqrt(accX * accX + accZ * accZ)) / M_PI;
    pitch = D180 * std::atan(accX / std::sqrt(accY * accY + accZ * accZ)) / M_PI;
    roll = D180 * std::atan(accY / std::sqrt(accX * accX + accZ * accZ)) / M_PI;

//    float gyroXrate = gyroX / 131.0; // Convert to deg/s
//    float kalAngleX = kalmanX.getAngle(roll, gyroXrate, dt); // Calculate the angle using a Kalman filter
//
    float filtered_pitch = filter.filter(pitch);

    M5.Lcd.setCursor(0, 80);
    M5.Lcd.printf("yaw:   % 5.3f", yaw);
    M5.Lcd.setCursor(0, 100);
    M5.Lcd.printf("pitch: % 5.3f", pitch);
    M5.Lcd.setCursor(0, 120);
    M5.Lcd.printf("roll:  % 5.3f", roll);

    M5.Lcd.setTextSize(8);
    M5.Lcd.setCursor(20, 180);
    M5.Lcd.printf("%+03.3f    ", filtered_pitch);
    delay(500);
#endif

//    M5.Lcd.setCursor(180, 120);
//    M5.Lcd.printf("% 5.2f", kalAngleX);


}