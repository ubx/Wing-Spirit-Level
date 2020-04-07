//
// Created by andreas on 07.04.20.
//
#include <M5Stack.h>
#include "audio.h"

#ifdef USE_TTS

#include <ESP8266SAM.h>  //https://github.com/earlephilhower/ESP8266SAM
#include <AudioOutputI2S.h>  //https://github.com/earlephilhower/ESP8266Audio

auto *sam = new ESP8266SAM;
AudioOutputI2S *out = new AudioOutputI2S(0, 1, 32);

void Say(float roll_diff, float roll_torerance) {
    if (roll_diff > roll_torerance) {
        out->begin();
        sam->Say(out, "Lower!");
        out->stop();
    } else if (roll_diff < -roll_torerance) {
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

#else

void Say(float roll_diff, float roll_torerance) {
    if (roll_diff > roll_torerance) {
        M5.Speaker.tone(3000, 100);
    } else if (roll_diff < -roll_torerance) {
        M5.Speaker.tone(1500, 100);
    }
}

void Say(const char *str) {
    M5.Speaker.beep();
}


#endif
