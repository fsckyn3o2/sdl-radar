#include "radar.h"
#include <SDL2_gfxPrimitives.h>
#include <SDL2/SDL.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define RADAR_CENTER(radar) (radar->padding + radar->radius)

void radar_init(Radar *radar) {
    radar->trail_history = (RadarTrailPoint**) malloc(sizeof(RadarTrailPoint*) * radar->trail_larger);
    for (size_t i = radar->max_trail_length-1; i > 0; --i) {
        for (size_t n = 0; n < radar->trail_larger; ++n) {
            radar->trail_history[n] = (RadarTrailPoint*) malloc(sizeof(RadarTrailPoint) * radar->max_trail_length);
            radar->trail_history[n][i] = (RadarTrailPoint){0,0};
        }
    }
}

void radar_render(Radar *radar) {
    if (radar->renderTexture != NULL) {
        SDL_SetRenderTarget(radar->renderer, NULL);
        SDL_RenderCopy(radar->renderer, radar->renderTexture, NULL, &radar->destination);
    }
}

void radar_draw(Radar *radar) {

    if (radar->renderTexture == NULL) {
        radar->renderTexture = SDL_CreateTexture(
            radar->renderer,
            SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
            radar_width(radar), radar_height(radar)
        );
    }

    SDL_SetRenderTarget(radar->renderer, radar->renderTexture);
    SDL_RenderClear(radar->renderer);

    if (radar->with_grid) {
        radar_draw_bkg_grid(radar);
    }
    radar_draw_circles(radar);
    radar_draw_sweep_line(radar);
    update_radar_trail(radar);
    radar_draw_middle_point(radar);

    // Update angle
    radar->angle += radar->speed*radar->direction;
    if (radar->angle >= 360.0) {
        radar->angle = 0.0;
    }

    SDL_SetRenderTarget(radar->renderer, NULL);
}

void radar_draw_middle_point(Radar *radar) {
    // Draw middle circle
    RadarCenterPoint* cpz = &radar->centerPoint;
    roundedBoxRGBA(radar->renderer,
        RADAR_CENTER(radar)-cpz->radius,RADAR_CENTER(radar)-cpz->radius,
        RADAR_CENTER(radar)+cpz->radius, RADAR_CENTER(radar)+cpz->radius,
        cpz->corner, radar->sweepLineColor.r, radar->sweepLineColor.g, radar->sweepLineColor.b, radar->sweepLineColor.a);
}

void radar_draw_sweep_line(Radar *radar) {
    // Draw a sweep line
    double rad = radar->angle * M_PI / 180.0f;
    thickLineRGBA(radar->renderer,
                      RADAR_CENTER(radar),RADAR_CENTER(radar),
                      RADAR_CENTER(radar) + radar->radius * cos(rad),
                      RADAR_CENTER(radar) + radar->radius * sin(rad),
                      5, radar->sweepLineColor.r, radar->sweepLineColor.g, radar->sweepLineColor.b, radar->sweepLineColor.a);

    // rad -= 0.05;
    // thickLineRGBA(radar->renderer,
    //                   RADAR_CENTER(radar)+7,RADAR_CENTER(radar),
    //                   RADAR_CENTER(radar)+7 + radar->radius * cos(rad),
    //                   RADAR_CENTER(radar) + radar->radius * sin(rad),
    //                   20, radar->sweepLineColor.r, radar->sweepLineColor.g, radar->sweepLineColor.b, radar->sweepLineColor.a/4);
}

void update_radar_trail(Radar* radar) {
    for (size_t i = radar->max_trail_length-1; i > 0; --i) {
        for (size_t n = 0; n < radar->trail_larger; ++n) {
            radar->trail_history[n][i] = radar->trail_history[n][i-1];
        }
    }

    double rad = radar->angle * M_PI / 180.0f;
    for (size_t n = 0; n < radar->trail_larger; ++n) {

        radar->trail_history[n][0].x = RADAR_CENTER(radar) + cos(rad) * (radar->radius-n);
        radar->trail_history[n][0].y = RADAR_CENTER(radar) + sin(rad) * (radar->radius-n);

        for (size_t i = 0; i+1 < (radar->max_trail_length-1); ++i) {

            if (radar->trail_history[n][i+1].x == 0 || radar->trail_history[n][i+1].y == 0) {
                break;
            }

            Uint8 alpha = (Uint8)(255.0f * (radar->max_trail_length - i) / radar->max_trail_length);

            thickLineRGBA(radar->renderer,
            radar->trail_history[n][i].x,
            radar->trail_history[n][i].y,
            radar->trail_history[n][i+1].x,
            radar->trail_history[n][i+1].y,
            1, radar->trailColor.r, radar->trailColor.g, radar->trailColor.b, alpha);
        }
    }
}

void radar_draw_circles(const Radar *radar) {
    SDL_SetRenderDrawColor(radar->renderer, radar->color.r, radar->color.g, radar->color.b, radar->color.a);
    int count=0;
    for (int r = radar->radius; r > 0; r -= 100) {
        if(count == 0) {
            for (int rr = -3; rr < 3; rr+=1) {
                aacircleRGBA(radar->renderer, RADAR_CENTER(radar), RADAR_CENTER(radar), r+rr, radar->color.r, radar->color.g, radar->color.b, radar->color.a);
            }
        } else if (r <= 100){
            aacircleRGBA(radar->renderer, RADAR_CENTER(radar), RADAR_CENTER(radar), r, radar->color.r, radar->color.g, radar->color.b, radar->color.a);
        } else {
            aacircleRGBA(radar->renderer, RADAR_CENTER(radar), RADAR_CENTER(radar), r-3, radar->color.r, radar->color.g, radar->color.b, radar->color.a);
            aacircleRGBA(radar->renderer, RADAR_CENTER(radar), RADAR_CENTER(radar), r+3, radar->color.r, radar->color.g, radar->color.b, radar->color.a);
        }
        count++;
    }
}

void radar_draw_bkg_grid(const Radar *radar) {
    // Draw normal grid lines
    SDL_SetRenderDrawColor(radar->renderer, radar->grid.color.r, radar->grid.color.g, radar->grid.color.b, radar->grid.color.a);

    const int MAX = 2*radar->radius+2*radar->padding;

    // Draw horizontal lines
    int countG=0;
    for (int y = 0; y < MAX; y += radar->grid.cellSize) {
        SDL_RenderDrawLine(radar->renderer, 0, y, MAX, y);
        if (radar->grid.thinCellNumber > 0 && countG % radar->grid.thinCellNumber == 0) {
            SDL_RenderDrawLine(radar->renderer, 0, y+1, MAX, y+1);
            SDL_RenderDrawLine(radar->renderer, 0, y-1, MAX, y-1);
        }
        countG++;
    }
    SDL_RenderDrawLine(radar->renderer, 0, MAX-1, MAX, MAX-1);

    // Draw vertical lines
    countG=0;
    for (int x = 0; x < MAX; x += radar->grid.cellSize) {
        SDL_RenderDrawLine(radar->renderer, x, 0, x, MAX);
        if (radar->grid.thinCellNumber > 0 && countG % radar->grid.thinCellNumber == 0) {
            SDL_RenderDrawLine(radar->renderer, x+1, 0, x+1, MAX);
            SDL_RenderDrawLine(radar->renderer, x-1, 0, x-1, MAX);
        }
        countG++;
    }
    SDL_RenderDrawLine(radar->renderer, MAX-1, 0, MAX-1, MAX);
}

int radar_width(const Radar *radar) {
    return radar->radius*2+radar->padding*2;
}

int radar_height(const Radar *radar) {
    return radar->radius*2+radar->padding*2;
}

SDL_Rect radar_rectangle(const Radar *radar, int x, int y) {
    return (SDL_Rect){x, y, radar_width(radar), radar_height(radar)};
}

SDL_Rect radar_rectangle_centered(const Radar *radar, int x, int y) {
    return (SDL_Rect){x-radar_width(radar)/2, y-radar_height(radar)/2, radar_width(radar), radar_height(radar)};
}