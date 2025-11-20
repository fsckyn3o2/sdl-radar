#ifndef RADAR_OBJECT_H
#define RADAR_OBJECT_H
#include "radar.h"

void radar_object_list_add(RadarObjectLinkedList *objectLst, RadarObject radarObject);
void radar_object_list_clear(RadarObjectLinkedList *objectLst);

void radar_object_list_anim_update(Radar *radar);
void radar_object_anim_update(const Radar *radar, RadarObject *radarObject);
bool radar_object_isIn(Radar *radar, RadarObject *object);
void radar_object_anim_destroy(RadarObject *radarObject);
void radar_object_list_anim_render(const Radar *radar);
void radar_object_anim_render(const Radar *radar, RadarObject *radarObject);

RadarObjectLinkedList* radar_object_generate_random_list(Radar *radar);

#endif