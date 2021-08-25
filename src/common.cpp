#include <cmath>
#include <Arduino.h>
#include "common.h"
//
// Created by andreas on 13.04.20.
//

float calcYay(const float accX, const float accY, const float accZ) {
    //D180 * atan(accZ / sqrt(accX * accX + accZ * accZ)) / M_PI;
    float t = accX * accX + accZ * accZ;
    if (t == 0.0) return t;
    return RAD_TO_DEG * std::atan(accZ / std::sqrt(t));
}

float calcPitch(const float accX, const float accY, const float accZ) {
    //  D180 * atan(accX / sqrt(accY * accY + accZ * accZ)) / M_PI;
    float t = accY * accY + accZ * accZ;
    if (t == 0.0) return t;
    return RAD_TO_DEG * std::atan(accX / std::sqrt(t));
}

float calcRoll(const float accX, const float accY, const float accZ) {
    // D180 * atan(accY / sqrt(accX * accX + accZ * accZ)) / M_PI;
    float t = accX * accX + accZ * accZ;
    if (t == 0.0) return t;
    return RAD_TO_DEG * std::atan(accY / std::sqrt(t));
}
