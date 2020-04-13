#include <cmath>

//
// Created by andreas on 06.04.20.
//

#ifndef WING_SPIRIT_LEVEL_MESSAGE_DIF_H
#define WING_SPIRIT_LEVEL_MESSAGE_DIF_H

#define D180 180.0
#define ALPHA 0.4

typedef struct {
    float pitch;
    float roll;
    float yaw;
    float filtered_pitch;
} struct_message;

float calcYay(const float accX, const float accY, const float accZ) {
    //D180 * atan(accZ / sqrt(accX * accX + accZ * accZ)) / M_PI;
    float t = accX * accX + accZ * accZ;
    if (t == 0.0) return t;
    return D180 * atan(accZ / std::sqrt(t)) / M_PI;
}

float calcPitch(const float accX, const float accY, const float accZ) {
    //  D180 * atan(accX / sqrt(accY * accY + accZ * accZ)) / M_PI;
    float t = accY * accY + accZ * accZ;
    if (t == 0.0) return t;
    return D180 * atan(accX / std::sqrt(t)) / M_PI;
}

float calcRoll(const float accX, const float accY, const float accZ) {
    // D180 * atan(accY / sqrt(accX * accX + accZ * accZ)) / M_PI;
    float t = accX * accX + accZ * accZ;
    if (t == 0.0)
        return t;
    return D180 * atan(accY / std::sqrt(t)) / M_PI;
}

#endif //WING_SPIRIT_LEVEL_MESSAGE_DIF_H
