#include <SDL2/SDL.h>
#include <stdbool.h>
#include "main_constants.h"
#include "radar.h"
#include "radar_sphere.h"

int main(void) {
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    bool running = true;

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL initialization failed: %s\n", SDL_GetError());
        return 1;
    }

    // Create a window
    window = SDL_CreateWindow("RADAR",
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
        .trail_history_index = 0
    };

    radar_init(&radar);
    radar.destination = radar_rectangle_centered(&radar, CENTER_X, CENTER_Y);

    // Main loop
    float angle_y = 0.0f;
    float angle_x = 0.0f;
    float offset = 10.0f;
    int mode = 0;

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_v || event.key.keysym.sym == SDLK_r) {
                    mode = mode?0:1;
                    SDL_DestroyTexture(radar.renderedTexture);
                    radar.renderedTexture = NULL;
                }

                if ((event.key.keysym.mod & KMOD_CTRL) != 0) {
                    offset = 1.0f;
                } else {
                    offset = 10.0f;
                }

                if (mode)
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

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        radar_draw(&radar);
        if (mode) {
            render_uv_mapped_sphere(&radar, angle_y, angle_x);
        }
        radar_render(&radar);

        // Present render
        SDL_RenderPresent(renderer);

        // Add small delay to control frame rate
        SDL_Delay(5);  // Approximately 60 FPS
    }

    // Cleanup
    radar_cleanup(&radar);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
