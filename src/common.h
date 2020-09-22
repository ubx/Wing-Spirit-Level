//
// Created by andreas on 06.04.20.
//

#ifndef WING_SPIRIT_LEVEL_MESSAGE_DIF_H
#define WING_SPIRIT_LEVEL_MESSAGE_DIF_H

#define ALPHA 0.4
#define ALPHA_DIFF 0.05

typedef struct {
    float pitch;
    float roll;
    float yaw;
    float filtered_pitch;
} struct_message;

float calcYay(float accX, float accY, float accZ);

float calcPitch(float accX, float accY, float accZ);

float calcRoll(float accX, float accY, float accZ);

#endif //WING_SPIRIT_LEVEL_MESSAGE_DIF_H
