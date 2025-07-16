#include "radar.h"
#include <SDL2_gfxPrimitives.h>
#include <SDL2/SDL.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define RADAR_CENTER(radar) (radar->padding + radar->radius)

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

    // Update angle
    radar->angle += radar->speed;
    if (radar->angle >= 360.0) {
        radar->angle = 0.0;
    }

    SDL_SetRenderTarget(radar->renderer, NULL);
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

    // Draw middle circle
    RadarCenterPoint* cpz = &radar->centerPoint;
    roundedBoxRGBA(radar->renderer,
        RADAR_CENTER(radar)-cpz->radius,RADAR_CENTER(radar)-cpz->radius,
        RADAR_CENTER(radar)+cpz->radius, RADAR_CENTER(radar)+cpz->radius,
        cpz->corner, radar->sweepLineColor.r, radar->sweepLineColor.g, radar->sweepLineColor.b, radar->sweepLineColor.a);
}

void update_radar_trail(Radar* radar) {

    for (size_t i = radar->max_trail_length-1; i > 0; --i) {
        radar->trail_history[i] = radar->trail_history[i-1];
    }

    // Calculer la position actuelle de la pointe (conversion angle double en int x, y)
    double rad = radar->angle * M_PI / 180.0f;
    int tipX = RADAR_CENTER(radar) + cos(rad) * radar->radius;
    int tipY = RADAR_CENTER(radar) + sin(rad) * radar->radius;
    radar->trail_history[0].x = tipX;
    radar->trail_history[0].y = tipY;

    for (size_t i = 0; i+1 < (radar->max_trail_length-1); ++i) {

        if (radar->trail_history[i+1].x == 0 || radar->trail_history[i+1].x == 0) {
            break;
        }

        Uint8 alpha = (Uint8)(255.0f * (radar->max_trail_length - i) / radar->max_trail_length);

        thickLineRGBA(radar->renderer,
        radar->trail_history[i].x,
        radar->trail_history[i].y,
        radar->trail_history[i+1].x,
        radar->trail_history[i+1].y,
        15, radar->sweepLineColor.r, radar->sweepLineColor.g, radar->sweepLineColor.b, alpha);
    }
}

void radar_draw_circles(const Radar *radar) {
    // Draw radar circles
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