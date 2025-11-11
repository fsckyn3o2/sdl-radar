#ifndef RADAR_H
#define RADAR_H
#include <SDL2/SDL.h>

// Structure simple pour stocker les points de la traînée
typedef struct {
    int x, y;
} RadarTrailPoint;

typedef struct {
    SDL_Color color;
    int cellSize;
    int thinCellNumber;
} RadarGrid;

typedef struct {
    int radius;
    int corner;
} RadarCenterPoint;

typedef struct {
    int direction;
    SDL_Rect destination;
    int radius;
    int padding;
    int with_grid;
    double angle;
    double speed;
    RadarCenterPoint centerPoint;
    SDL_Color color;
    SDL_Color sweepLineColor;
    RadarGrid grid;
    SDL_Renderer *renderer;
    SDL_Texture *workingTexture;
    SDL_Texture *renderedTexture;;
    SDL_Color trailColor;
    int trail_history_index;
    int trail_larger;
    int max_trail_length;
    RadarTrailPoint **trail_history;
} Radar;

void radar_init(Radar *radar);
void radar_render(Radar *radar);
void radar_draw(Radar *radar);
void radar_draw_middle_point(Radar *radar);
void radar_draw_sweep_line(Radar *radar);
void update_radar_trail(Radar* radar);
void radar_draw_circles(const Radar *radar);
void radar_draw_bkg_grid(const Radar *radar);

SDL_Rect radar_position(const Radar *radar);
SDL_Point radar_center(const Radar *radar);
SDL_Rect radar_rectangle(const Radar *radar, int x, int y);
SDL_Rect radar_rectangle_centered(const Radar *radar, int x, int y);
int radar_width(const Radar *radar);
int radar_height(const Radar *radar);

void radar_cleanup(Radar *radar);
#endif