// Bart Middag & Thomas Dhondt ; groepsnummer: 14 ; world.h

#ifndef H_WORLD
#define H_WORLD

#include "entities.h"

typedef struct {
	int width_in_tiles;
	int height_in_tiles;
	Entity ** entities;
	Entity * spawn;
	Entity * castle;
} World;

void init_world(World * world, int width_in_tiles, int heigth_in_tiles);

void init_world_from_file(World * world, char * worldFile);

Entity * place_tower(World * world, TowerType type, float world_x, float world_y);

void destroy_tower(World * world, Entity * tower);

void destroy_world(World * world);

#endif