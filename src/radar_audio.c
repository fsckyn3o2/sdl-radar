#include "radar_audio.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <math.h>
#include <stdatomic.h>
#include <string.h>
#include <stdbool.h>

void radar_audio_callback(void* userdata, Uint8* stream, int len) {
    RadarAudioUserData* audio = userdata;

    Sint16* buffer = (Sint16*)stream;
    if (SDL_FALSE == audio->playing) {
        SDL_memset(stream, 0, len);
        return;
    }

    int samples_to_fill = len / sizeof(Sint16);
    for (int i = 0; i < samples_to_fill; ++i) {
        if (audio->samples_left > 0) {
            // Generate a sine wave sample
            buffer[i] = (Sint16)(AMPLITUDE * sin(audio->phase));
            // Update phase for the next sample
            audio->phase += (2.0 * M_PI * audio->frequency) / SAMPLE_RATE;
            while (audio->phase >= (2.0 * M_PI)) {
                audio->phase -= (2.0 * M_PI);
            }
            audio->samples_left--;
        } else {
            buffer[i] = 0; // Silence when the sound effect finishes
            audio->playing = SDL_FALSE;
        }
    }
}

void radar_audio_init(Radar *radar) {

    radar->audioData.initialized = 0;
    SDL_zero(radar->audioData.desiredSpec);
    radar->audioData.desiredSpec.freq = SAMPLE_RATE;
    radar->audioData.desiredSpec.format = AUDIO_S16SYS; // System-dependent 16-bit signed integer format
    radar->audioData.desiredSpec.channels = 1; // Mono sound
    radar->audioData.desiredSpec.samples = 4096; // Buffer size
    radar->audioData.desiredSpec.callback = radar_audio_callback;
    radar->audioData.desiredSpec.userdata = &radar->audioData.userData;

    radar->audioData.deviceId = SDL_OpenAudioDevice(NULL, 0, &radar->audioData.desiredSpec, &radar->audioData.actualSpec, 0);
    if (radar->audioData.deviceId == 0) {
        fprintf(stderr, "Failed to open audio device: %s\n", SDL_GetError());
        radar->audioData.initialized = 0;
    } else {
        printf("Audio device opened successfully!\n");
        printf("Actual frequency: %d Hz, Actual format: %u, Actual channels: %d\n",
                    radar->audioData.actualSpec.freq, radar->audioData.actualSpec.format, radar->audioData.actualSpec.channels);
    }


    // Initialize audio data struct manually
    radar->audioData.userData.frequency = 0.0;
    radar->audioData.userData.duration_ms = 0.0;
    radar->audioData.userData.samples_left = 0;
    radar->audioData.userData.phase = 0.0;
    radar->audioData.userData.playing = SDL_FALSE;

    // Start playing audio (unpause)
    SDL_PauseAudioDevice(radar->audioData.deviceId, 0);

    radar->audioData.initialized = 1;
}

void radar_audio_play(Radar *radar) {
    if (SDL_FALSE == radar->audioData.userData.playing) {
        SDL_LockAudioDevice(radar->audioData.deviceId); // Lock audio to safely modify shared data
        radar->audioData.userData.frequency = 1200.0; // M_PIng frequency (Hz)
        radar->audioData.userData.duration_ms = 150.0; // M_PIng duration (milliseconds)
        radar->audioData.userData.samples_left = (int)((radar->audioData.userData.duration_ms / 1000.0) * SAMPLE_RATE);
        radar->audioData.userData.phase = 0.0;
        radar->audioData.userData.playing = SDL_TRUE;
        SDL_UnlockAudioDevice(radar->audioData.deviceId); // Unlock audio
    }
}

void radar_audio_cleanup(Radar *radar) {
    printf("Radar audio cleanup\n");
    SDL_CloseAudioDevice(radar->audioData.deviceId);
}

void radar_audio_trigger(Radar *radar) {
    // Detect collision between the radar line and a dot point on the radar zone

}

int radar_audio_thread(void* radarP) {
    Radar *radar = (Radar*) radarP;
    atomic_init(&radar->audioData.audioThreadRunning, true);
    printf("Audio thread started successfully!\n");
    while (atomic_load(&radar->audioData.audioThreadRunning) == true) {
        radar_audio_trigger(radar);
        SDL_Delay(10); // Sleep briefly to prevent 100% CPU usage
    }
    printf("Audio thread stopped successfully!\n");

    return 0;
}
