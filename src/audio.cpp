//
// Created by andreas on 07.04.20.
//
#include "audio.h"

#ifdef USE_TTS

#include <ESP8266SAM.h>  //https://github.com/earlephilhower/ESP8266SAM
#include <AudioOutputI2S.h>  //https://github.com/earlephilhower/ESP8266Audio

auto *sam = new ESP8266SAM;
AudioOutputI2S *out = new AudioOutputI2S(0, 1, 32);

void Say(float roll_diff) {
    if (roll_diff > PITCH_TOLERANCE) {
        out->begin();
        sam->Say(out, "Lower!");
        out->stop();
    } else if (roll_diff < -PITCH_TOLERANCE) {
        out->begin();
        sam->Say(out, "Higher!");
        out->stop();
    }
}

void Say(const char *str) {
    out->begin();
    sam->Say(out, str);
    out->stop();
}

#endif
