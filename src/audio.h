//
// Created by andreas on 07.04.20.
//

#ifndef WING_SPIRIT_LEVEL_AUDIO_H
#define WING_SPIRIT_LEVEL_AUDIO_H

#define USE_TTS
#define PITCH_TOLERANCE 5.0F

void Say(float roll);
void Say(const char *str);

#endif //WING_SPIRIT_LEVEL_AUDIO_H
