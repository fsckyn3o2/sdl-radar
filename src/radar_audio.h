#ifndef RADAR_AUDIO_H
#define RADAR_AUDIO_H
#include "radar.h"

#define SAMPLE_RATE 44100
#define AMPLITUDE 10000
#define PING_DURATION_MS 250
#define REVERB_DELAY_MS 120
#define DECAY_FACTOR 0.5f // How much each echo fades
#define PING_FREQ_START 1300.0
#define PING_FREQ_END 1800.0

void radar_audio_callback(void* userdata, Uint8* stream, int len);
void radar_audio_play(Radar *radar);
void radar_audio_cleanup(Radar *radar);
void radar_audio_init(Radar *radar);
void radar_audio_trigger(Radar *radar);
int radar_audio_thread(void *radarP);

#endif