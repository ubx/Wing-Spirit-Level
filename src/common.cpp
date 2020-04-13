#include <cmath>
#include "common.h"
//
// Created by andreas on 13.04.20.
//


float calcYay(const float accX, const float accY, const float accZ) {
    //D180 * atan(accZ / sqrt(accX * accX + accZ * accZ)) / M_PI;
    float t = accX * accX + accZ * accZ;
    if (t == 0.0) return t;
    return D180 * std::atan(accZ / std::sqrt(t)) / M_PI;
}

float calcPitch(const float accX, const float accY, const float accZ) {
    //  D180 * atan(accX / sqrt(accY * accY + accZ * accZ)) / M_PI;
    float t = accY * accY + accZ * accZ;
    if (t == 0.0) return t;
    return D180 * std::atan(accX / std::sqrt(t)) / M_PI;
}

float calcRoll(const float accX, const float accY, const float accZ) {
    // D180 * atan(accY / sqrt(accX * accX + accZ * accZ)) / M_PI;
    float t = accX * accX + accZ * accZ;
    if (t == 0.0)
        return t;
    return D180 * std::atan(accY / std::sqrt(t)) / M_PI;
}
