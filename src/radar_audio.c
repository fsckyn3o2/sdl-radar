#include "radar_audio.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <math.h>
#include <stdatomic.h>
#include <string.h>
#include <stdbool.h>

void radar_audio_callback(void* userdata, Uint8* stream, int len) {
    RadarAudioUserData *audio_data = (RadarAudioUserData*) userdata;

    if (SDL_FALSE == audio_data->playing) {
        SDL_memset(stream, 0, len);
        return;
    }

    Sint16 *snd = (Sint16 *)stream;
    int sample_count = len / sizeof(Sint16);
    static double phase = 0.0;

    for (int i = 0; i < sample_count; i++) {
        Sint16 current_sample = 0;

        // Generate the main ping sound if active
        if (audio_data->samples_left > 0) {
            // Simple frequency sweep for a "ping" effect
            double current_freq = PING_FREQ_START + (double)(audio_data->ping_length_samples - audio_data->samples_left) * (PING_FREQ_END - PING_FREQ_START) / audio_data->ping_length_samples;
            phase += (current_freq * 2.0 * M_PI) / SAMPLE_RATE;
            if (phase >= 2.0 * M_PI) phase -= 2.0 * M_PI;

            current_sample = (Sint16)(AMPLITUDE * sin(phase));
            audio_data->samples_left--;
        } else {
            audio_data->playing = SDL_FALSE;
        }

        // Add simple echo (reverb)
        if (audio_data->reverb_buffer) {
            Sint16 echo_sample = audio_data->reverb_buffer[audio_data->reverb_buffer_pos];
            // Add the echo to the current sample
            current_sample += (Sint16)(echo_sample * DECAY_FACTOR);

            // Store the current (mixed) sample for future echoes
            audio_data->reverb_buffer[audio_data->reverb_buffer_pos] = current_sample;

            // Move the buffer position
            audio_data->reverb_buffer_pos++;
            if (audio_data->reverb_buffer_pos >= audio_data->reverb_buffer_size) {
                audio_data->reverb_buffer_pos = 0;
            }
        }


        snd[i] = current_sample;
    }
}
// void radar_audio_callback(void* userdata, Uint8* stream, int len) {
//     RadarAudioUserData* audio = userdata;
//
//     Sint16* buffer = (Sint16*)stream;
//     if (SDL_FALSE == audio->playing) {
//         SDL_memset(stream, 0, len);
//         return;
//     }
//
//     int samples_to_fill = len / sizeof(Sint16);
//     for (int i = 0; i < samples_to_fill; ++i) {
//         if (audio->samples_left > 0) {
//             if (audio->samples_left <= audio->silence_left) {
//                 buffer[i] = 0;
//                 audio->samples_left--;
//                 continue;
//             }
//             // Generate a sine wave sample
//             buffer[i] = (Sint16)(AMPLITUDE * sin(audio->phase));
//             // Update phase for the next sample
//             audio->phase += (2.0 * M_PI * audio->frequency) / SAMPLE_RATE;
//             while (audio->phase >= (2.0 * M_PI)) {
//                 audio->phase -= (2.0 * M_PI);
//             }
//             audio->samples_left--;
//         } else {
//             buffer[i] = 0; // Silence when the sound effect finishes
//             audio->playing = SDL_FALSE;
//         }
//     }
// }

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
    radar->audioData.userData.ping_length_samples = (SAMPLE_RATE * PING_DURATION_MS) / 1000;
    radar->audioData.userData.reverb_buffer_size = (SAMPLE_RATE * REVERB_DELAY_MS) / 1000;
    radar->audioData.userData.reverb_buffer = (Sint16*)calloc(radar->audioData.userData.reverb_buffer_size, sizeof(Sint16));

    // Start playing audio (unpause)
    SDL_PauseAudioDevice(radar->audioData.deviceId, 0);

    radar->audioData.initialized = 1;
}

void radar_audio_play(Radar *radar) {
    if (SDL_FALSE == radar->audioData.userData.playing) {
        SDL_LockAudioDevice(radar->audioData.deviceId); // Lock audio to safely modify shared data
        radar->audioData.userData.frequency = 750.0; // M_PIng frequency (Hz)
        radar->audioData.userData.duration_ms = 250.0; // M_PIng duration (milliseconds)
        radar->audioData.userData.silence_ms = 150.0;
        radar->audioData.userData.samples_left = (int)(( (radar->audioData.userData.duration_ms+radar->audioData.userData.silence_ms) / 1000.0) * SAMPLE_RATE);
        radar->audioData.userData.silence_left = (int)((radar->audioData.userData.silence_ms / 1000.0) * SAMPLE_RATE);
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
    if (radar->radar_objects == NULL) return;

    RadarObjectLinkedList *radarObject = radar->radar_objects;
    double rad = radar->angle * M_PI / 180.0f;
    int radarLx=cos(rad)*radar->radius;
    int radarLy=sin(rad)*radar->radius;
    do {
        if (
            (
                (radarLx < 0 && radarObject->object.x >= radarLx && radarObject->object.x <= 0) ||
                (radarLx >= 0 && radarObject->object.x <= radarLx && radarObject->object.x >= 0)
            ) &&
            (
                (radarLy < 0 && radarObject->object.y >= radarLy && radarObject->object.y <= 0) ||
                (radarLy >= 0 && radarObject->object.y <= radarLy && radarObject->object.y >= 0)
            )
        ){
            radar_audio_play(radar);
        }
    }while((radarObject = radarObject->next) != NULL);
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
