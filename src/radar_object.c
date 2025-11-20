#include "radar_object.h"
#include <SDL2_gfxPrimitives.h>
#include <stdlib.h>
#include <time.h>

void radar_object_list_add(RadarObjectLinkedList *objectLst, RadarObject radarObject) {
    while (objectLst->next != NULL) {
        objectLst = objectLst->next;
    }
    objectLst->next = malloc(sizeof(RadarObjectLinkedList));
    objectLst->next->object = radarObject;
    objectLst->next->next = NULL;
}

void radar_object_list_clear(RadarObjectLinkedList *objectLst) {
    RadarObjectLinkedList *tmp = objectLst;
    while (objectLst->next != NULL) {
        while (tmp->next != NULL) {
            tmp = tmp->next;
        }
        free(tmp->next);
    }
}

void radar_object_list_anim_update(Radar *radar) {
    if (radar->radar_objects == NULL) { return; }

    RadarObjectLinkedList *objectLst = radar->radar_objects;
    RadarObjectLinkedList *prevObj = NULL;
    do{
        switch (objectLst->object.status) {
            case RADAR_OBJECT_STATUS_DEAD:
                if (prevObj == NULL) {
                    if (objectLst->next == NULL) {
                        radar->radar_objects = NULL;
                    } else {
                        radar->radar_objects = objectLst->next;
                    }
                    free(objectLst);
                    objectLst = radar->radar_objects;
                } else {
                    prevObj->next = objectLst->next;
                    free(objectLst);
                    objectLst = prevObj;
                }
                break;
            case RADAR_OBJECT_STATUS_IS_DYING:
                radar_object_anim_destroy(&objectLst->object);
            case RADAR_OBJECT_STATUS_ALIVE:
                radar_object_anim_update(radar, &objectLst->object);
            default:
                break;
        }
        prevObj = objectLst;
    } while(objectLst != NULL && (objectLst = objectLst->next) != NULL);
}

void radar_object_anim_update(const Radar *radar, RadarObject *radarObject) {
    double offsetX = radarObject->speed * cos(radarObject->directionAngle);
    double offsetY = radarObject->speed * sin(radarObject->directionAngle);
    radarObject->x += offsetX>-1 && offsetX<0 ? -1 : offsetX>0 && offsetX<-1 ? 1 : offsetX;
    radarObject->y += offsetY>-1 && offsetY<0 ? -1 : offsetY>0 && offsetY<-1 ? 1 : offsetY;

    if (!radar_object_isIn(radar, radarObject)) {
        radarObject->status = RADAR_OBJECT_STATUS_DEAD;
    }
}

bool radar_object_isIn(Radar *radar, RadarObject *object) {
    return sqrt(pow(object->x,2) + pow(object->y,2)) < radar->radius;
}

void radar_object_anim_destroy(RadarObject *radarObject) {
    if(radarObject->status == RADAR_OBJECT_STATUS_IS_DYING) {
        radarObject->radius -= 1; // Decrease size gradually
        if (radarObject->radius <= 0) {
            radarObject->radius = radarObject->radius_memory;
            radarObject->radius_memory = radarObject->radius_memory/2;
            if (radarObject->radius_memory <= 2) {
                radarObject->status = RADAR_OBJECT_STATUS_DEAD;
            }
        }
    }
}

void radar_object_list_anim_render(const Radar *radar) {
    if (radar->radar_objects==NULL) return;

    RadarObjectLinkedList *objectLst = radar->radar_objects;
    SDL_SetRenderTarget(radar->renderer, radar->workingTexture);
    do{
        radar_object_anim_render(radar, &objectLst->object);
    }while((objectLst = objectLst->next) != NULL);
}

void radar_object_anim_render(const Radar *radar, RadarObject *radarObject) {
    if (radarObject == NULL || radarObject->status != RADAR_OBJECT_STATUS_ALIVE)
        return;

    SDL_Renderer* renderer = radar->renderer;
    // Determine color based on an object type
    SDL_Color color;
    if (radarObject->type < 0) { // Negative types are enemies
        switch (radarObject->type) {
            case ENEMY_DRONE: color = (SDL_Color){255, 0, 0, 128}; break; // red with transparency
            case ENEMY_TANK: color = (SDL_Color){255, 100, 0, 128}; break; // orange
            case ENEMY_BOMBER: color = (SDL_Color){255, 50, 50, 128}; break; // pink
            case ENEMY_SNIPER: color = (SDL_Color){255, 0, 255, 128}; break; // magenta
            case ENEMY_ARTILLERY: color = (SDL_Color){255, 255, 0, 128}; break; // yellow
            case ENEMY_ARMORED: color = (SDL_Color){128, 128, 128, 128}; break; // grey
            case ENEMY_BOSSES: color = (SDL_Color){255, 0, 0, 128}; break; // red
            default: color = (SDL_Color){255,255,255,128}; break;
        }
    } else if (radarObject->type > 0) { // positive types are allies
        switch (radarObject->type) {
            case ALLY_SCOUT: color = (SDL_Color){0, 255, 0, 128}; break; // green
            case ALLY_MEDIC: color = (SDL_Color){0, 0, 255, 128}; break; // blue
            case ALLY_TANKER: color = (SDL_Color){0, 128, 255, 128}; break; // light-blue
            case ALLY_SNIPER_SUPPORT: color = (SDL_Color){0, 255, 255, 128}; break; // cyan
            case ALLY_TECHNICIAN: color = (SDL_Color){255, 255, 255, 128}; break; // white
            case ALLY_DRONE: color = (SDL_Color){0, 255, 255, 128}; break; // cyan
            case ALLY_COMMANDER: color = (SDL_Color){255, 255, 0, 128}; break; // yellow
            default: color = (SDL_Color){255,255,255,128}; break;
        }
    } else {
        color = (SDL_Color){0, 128, 0, 128};
    }

    // Draw a blur effect: multiple circles with decreasing alpha
    int layers_r = radarObject->radius/3;
    for (int i = 0; i <= radarObject->radius; i+=layers_r) {
        if (radarObject->radius-i >= radarObject->radius-layers_r) {
            filledCircleRGBA(renderer,  radarObject->x + radar->radius+radar->padding, radarObject->y + radar->radius+radar->padding, i, color.r, color.g, color.b, 255);
        }
        filledCircleRGBA(renderer,  radarObject->x + radar->radius+radar->padding, radarObject->y + radar->radius+radar->padding, i, color.r, color.g, color.b, color.a/3);
    }

    SDL_Color clearColor = {0, 0, 0, 0};
    SDL_SetRenderDrawColor(renderer, clearColor.r, clearColor.g, clearColor.b, clearColor.a);
}

RadarObjectLinkedList* radar_object_generate_random_list(Radar *radar) {
    const int NUM_OBJECTS = 10; // Adjust as needed
    RadarObjectLinkedList *head = malloc(sizeof(RadarObjectLinkedList));
    head->next = NULL;

    RadarObjectLinkedList *current = head;
    RadarObjectLinkedList *new_node = head;

    for (int i = 0; i < NUM_OBJECTS; ++i) {
        // Allocate a new RadarObject
        new_node->object = (RadarObject) {
            .x = rand() % radar->radius/2,
            .y = rand() % radar->radius/2,
            .radius = 16 + rand() % 8, // radius between 2 and 10
            .radius_memory = 0,
            .directionAngle = ((double)rand() / RAND_MAX) * 2 * M_PI,
            .speed = ((double) (rand()%(radar->radius/100))) * ((rand() % 2) * 2 - 1),
            .status = (rand() % 3) - 1, // -1, 0, or 1
            .type = -1
        };
        new_node->object.radius_memory = new_node->object.radius;

        int type_selector = rand() % 2;
        if (type_selector == 0) { // Enemy
            new_node->object.type = -1 * (rand() % 8);
        } else { // Ally
            new_node->object.type = (rand() % 8);
        }

        if (new_node != current) {
            current->next = new_node;
        }
        current = new_node;

        // Allocate a new node
        new_node = malloc(sizeof(RadarObjectLinkedList));
        new_node->next = NULL;
    }

    free(new_node);
    return head;
}