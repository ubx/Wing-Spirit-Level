//
// Created by andreas on 06.04.20.
//

#ifndef WING_SPIRIT_LEVEL_MESSAGE_DIF_H
#define WING_SPIRIT_LEVEL_MESSAGE_DIF_H

#define D180 180.0
#define ALPHA 0.5

typedef struct {
    float pitch;
    float roll;
    float yaw;
    float filtered_pitch;
} struct_message;
#endif //WING_SPIRIT_LEVEL_MESSAGE_DIF_H
