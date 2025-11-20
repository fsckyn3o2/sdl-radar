#include <SDL2/SDL.h>
#include "main_constants.h"
#include "radar.h"
#include "radar_audio.h"
#include "radar_sphere.h"
#include "radar_object.h"
#include <stdatomic.h>

int main(void) {
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL initialization failed: %s\n", SDL_GetError());
        return 1;
    }

    // Create a window
    window = SDL_CreateWindow("-- RADAR --",
                            SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED,
                            WINDOW_WIDTH, WINDOW_HEIGHT,
                            SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Window creation failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Create renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    if (!renderer) {
        printf("Renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Initialize audio
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        return 1;
    }

    Radar radar = {
        .renderer=renderer,
        .direction=-1, // Multiply by speed to get negative number or positive number, this direction parameter can use to speed up, reverse the rotation or stop the radar line.
        .angle=0.0,
        .destination= {0,0,0,0},
        .padding=0,
        .radius=RADAR_RADIUS,
        .with_grid=1,
        .speed=SWEEP_SPEED,
        .color = {100, 255, 100, 255},
        .centerPoint = {10, 10},
        .sweepLineColor = {255, 255, 255, 255},
        .grid = {
            .color = {100, 150, 100, 100},
            .cellSize = 40,
            .thinCellNumber = -1
        },
        .max_trail_length = 40,
        .trail_larger = RADAR_RADIUS,
        .trailColor =  {106, 220, 153, 255},
        .trail_history_index = 0,
        .audioData = {0}
    };

    radar_init(&radar);
    radar.destination = radar_rectangle_centered(&radar, CENTER_X, CENTER_Y);

    // OBJECTS on the radar :
    radar.radar_objects = radar_object_generate_random_list(&radar);

    // AUDIO: Create the new thread (Name the thread, pass the function, pass the user data struct)
    radar_audio_init(&radar);
    if (radar.audioData.initialized == 0) {
        printf("Error initialization");
        return 1;
    }

    // No Audio Thread
    // SDL_Thread* radarAudioThread = SDL_CreateThread(radar_audio_thread, "RadarAudioThread", &radar);
    //
    // if (radarAudioThread == NULL) {
    //     fprintf(stderr, "Failed to create thread: %s\n", SDL_GetError());
    //     radar_cleanup(&radar);
    //     SDL_Quit();
    //     return 1;
    // }

    // Main loop
    float angle_y = 0.0f;
    float angle_x = 0.0f;
    float offset = 10.0f;
    int mode = 0;

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                atomic_store(&radar.audioData.audioThreadRunning, false);
                running = false;
            } else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_v:
                    case SDLK_r:
                        mode = mode?0:1;
                        SDL_DestroyTexture(radar.renderedTexture);
                        radar.renderedTexture = NULL;
                        break;
                    default:
                        break;
                }

                if (mode) {
                    if ((event.key.keysym.mod & KMOD_CTRL) != 0) {
                        offset = 1.0f;
                    } else {
                        offset = 10.0f;
                    }
                    switch (event.key.keysym.sym) {
                        case SDLK_w:
                            angle_x += offset;
                            break;
                        case SDLK_a:
                            angle_y -= offset;
                            break;
                        case SDLK_s:
                            angle_x -= offset;
                            break;
                        case SDLK_d:
                            angle_y += offset;
                            break;
                        default:
                            break;
                    }
                }
            }
        }

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        radar_initWorkingTexture(&radar);

        radar_object_list_anim_update(&radar);
        radar_object_list_anim_render(&radar);

        radar_draw(&radar);
        if (mode) {
            render_uv_mapped_sphere(&radar, angle_y, angle_x);
        }
        radar_render(&radar);
        radar_audio_trigger(&radar);


        // Present render
        SDL_RenderPresent(renderer);

        // Add small delay to control frame rate
        SDL_Delay(5);  // Approximately 60 FPS
    }

    // Cleanup
    // No Audio Thread
    // SDL_WaitThread(radarAudioThread, NULL);
    radar_audio_cleanup(&radar);
    radar_cleanup(&radar);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
