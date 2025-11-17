#ifndef RADAR_AUDIO_H
#define RADAR_AUDIO_H
#include "radar.h"

#define SAMPLE_RATE 22050
#define AMPLITUDE 30000

void radar_audio_callback(void* userdata, Uint8* stream, int len);
void radar_audio_play(Radar *radar);
void radar_audio_cleanup(Radar *radar);
void radar_audio_init(Radar *radar);
void radar_audio_trigger(Radar *radar);
int radar_audio_thread(void *radarP);

#endif