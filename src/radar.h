#ifndef RADAR_H
#define RADAR_H
#include <SDL2/SDL.h>
#include <stdbool.h>
#define ASSET_TEXTURE_BLUR "asserts/blur.png"

typedef struct {
    double frequency;
    double duration_ms;
    double silence_ms;
    int samples_left;
    int silence_left;
    int ping_length_samples;
    int reverb_buffer_pos;
    int reverb_buffer_size;
    Sint16* reverb_buffer;
    double phase;
    SDL_bool playing;
} RadarAudioUserData;

typedef struct {
    RadarAudioUserData userData;
    int initialized;
    SDL_AudioSpec desiredSpec;
    SDL_AudioSpec actualSpec;
    SDL_AudioDeviceID deviceId;
    SDL_mutex *audioMutex;
    int audioThreadId;
    _Atomic(int) audioThreadRunning;
} RadarAudioData;

typedef struct {
    int x, y;
} RadarTrailPoint;

/**
* DEFAULT: Generic enemy
* DRONE: Small, fast enemy drone
* TANK: Heavy, slow-moving enemy vehicle
* BOMBER: Enemy that drops bombs or mines
* SNIPER: Enemy with long-range attack
* ARTILLERY: Long-range artillery unit
* ARMORED: Enemies with heavy armor
* BOSSES: Major enemies or bosses
*/
enum RadarEnemyType {
    ENEMY_DEFAULT = -1,
    ENEMY_DRONE = -2,
    ENEMY_TANK = -3,
    ENEMY_BOMBER = -4,
    ENEMY_SNIPER = -5,
    ENEMY_ARTILLERY = -6,
    ENEMY_ARMORED = -7,
    ENEMY_BOSSES = -8
};

/**
* DEFAULT: Generic ally
* SCOUT: Fast-moving ally for reconnaissance
* MEDIC: Provides healing or support
* TANKER: Heavy armor, protecting others
* SNIPER_SUPPORT: Long-range support ally
* TECHNICIAN: Repair or hacking units
* DRONE: Support drone or flying ally
* COMMANDER: Leader of allies
 */
enum RadarAllyType {
    ALLY_DEFAULT = 1,
    ALLY_SCOUT = 2,
    ALLY_MEDIC = 3,
    ALLY_TANKER = 4,
    ALLY_SNIPER_SUPPORT = 5,
    ALLY_TECHNICIAN = 6,
    ALLY_DRONE = 7,
    ALLY_COMMANDER = 8
};

enum RadarObjectStatus {
    RADAR_OBJECT_STATUS_DEAD = -1,
    RADAR_OBJECT_STATUS_ALIVE = 0,
    RADAR_OBJECT_STATUS_IS_DYING = 1
};

/**
 * Each object has the same origin which is the center of the radar.
 * Origin is:
 *   (depends on where the object is rendered (radar->destination.x)) + radar->padding + radar->radius
 *   (depends on where the object is rendered (radar->destination.y)) + radar->padding + radar->radius
 */
typedef struct {
    int x, y;
    int radius;
    int radius_memory;
    double directionAngle;
    double speed;
    int type;
    int status;
} RadarObject;

typedef struct RadarObjectLinkedList {
    struct RadarObjectLinkedList* next;
    RadarObject object;
} RadarObjectLinkedList;

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
    RadarAudioData audioData;
    RadarObjectLinkedList *radar_objects;
} Radar;

void radar_init(Radar *radar);
void radar_render(Radar *radar);
void radar_initWorkingTexture(Radar *radar);
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