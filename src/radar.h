#ifndef RADAR_H
#define RADAR_H
#include <SDL2/SDL.h>

// Maximum length the trail can ever be in the fixed-size C array
#define MAX_TRAIL_CAPACITY 2000

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
    SDL_Texture *renderTexture;

    // NOUVEAU CHAMP : Historique des points de la traînée
    RadarTrailPoint trail_history[MAX_TRAIL_CAPACITY];
    int trail_history_index;
    int max_trail_length; // Pour contrôler la longueur de la traînée
} Radar;

void radar_render(Radar *radar);
void radar_draw(Radar *radar);
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

#endif