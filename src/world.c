// Bart Middag & Thomas Dhondt ; groepsnummer: 14 ; world.c

#include <stdlib.h>
#include <stdio.h>
#include "entities.h"
#include "world.h"
#include "config.h"

void init_world(World * world, int width_in_tiles, int height_in_tiles)
{
	int i, j;

	world->width_in_tiles = width_in_tiles;
	world->height_in_tiles = height_in_tiles;
	world->entities = (Entity**) malloc(width_in_tiles*sizeof(Entity*));

	for(i = 0; i < width_in_tiles; i++) {
		//Hier doen we calloc, zodat alle waarden (die die we niet met init_entity behandelen) in het begin op 0 staan.
		world->entities[i] = (Entity*) calloc(height_in_tiles,sizeof(Entity));
		for(j = 0; j < height_in_tiles; j++) {
			init_entity(&world->entities[i][j],EMPTY,convert_tile2world_x(i),convert_tile2world_y(j),convert_tile2world_x(1),convert_tile2world_y(1));
		}
	}
	//Zet tijdelijk de spawn en castle entities op een willekeurige locatie.
	//Dit is in het geval dat world zou geïnitialiseerd worden zonder init_world_from_file.
	world->spawn = &world->entities[0][0];
	world->castle = &world->entities[width_in_tiles-1][height_in_tiles-1];
}

void init_world_from_file(World * world, char * worldFileName)
{
	FILE* worldFile;
	char line[20];
	int x, y, i;
	char type;
	worldFile = fopen(worldFileName, "r");
	//Geef fout als het inputbestand niet gevonden is.
	if(!worldFile){
		 fprintf(stderr,"[ERROR] Input file not found: %s\n", worldFileName);
		 return;
	}
	//Lees een lijn in van het bestand
	for(i = 0; fgets(line,20,worldFile) != NULL; i++){
		switch(i) {
		case 0:
			sscanf(line,"%d %d",&x,&y);
			init_world(world,x,y);
			break;
		case 1:
			sscanf(line,"%d %d",&x,&y);
			world->spawn = &world->entities[x][y];
			init_entity(world->spawn,SPAWN_LOCATION,convert_tile2world_x(x),convert_tile2world_y(y),convert_tile2world_x(1),convert_tile2world_y(1));
			init_frameAnimator(&world->spawn->spawn_location.frameAnimator, 3, 9, 5, 32, 32);
			break;
		case 2:
			sscanf(line,"%d %d",&x,&y);
			world->castle = &world->entities[x][y];
			init_entity(world->castle,CASTLE,convert_tile2world_x(x),convert_tile2world_y(y),convert_tile2world_x(1),convert_tile2world_y(1));
			init_frameAnimator(&world->castle->castle.frameAnimator, 3, 36, 4, 32, 32);
			break;
		default:
			sscanf(line,"%c %d %d",&type,&x,&y);
			init_entity(&world->entities[x][y],OBSTACLE,convert_tile2world_x(x),convert_tile2world_y(y),convert_tile2world_x(1),convert_tile2world_y(1));
			switch(type) {
			case 'M': world->entities[x][y].obstacle.obstacle_type = MOUNTAIN; break;
			case 'W': world->entities[x][y].obstacle.obstacle_type = WATER; break;
			default: break;
			}
			break;
		}
	}
}

Entity * place_tower(World * world, TowerType type, float world_x, float world_y)
{
	int x, y;
	x = convert_world2tile_x(world_x);
	y = convert_world2tile_y(world_y);
	switch(type) {
	case MACHINE_GUN: 
		init_machine_gun(&world->entities[x][y],world_x,world_y); return &world->entities[x][y];
	case ROCKET_LAUNCHER: 
		init_rocket_launcher(&world->entities[x][y],world_x,world_y); return &world->entities[x][y];
	case FLAK_CANNON: 
		init_flak_cannon(&world->entities[x][y],world_x,world_y); return &world->entities[x][y];
	default: 
		return NULL;
	}
}

void destroy_tower(World * world, Entity * tower)
{
	free(tower->tower.projectiles);
	tower->type = EMPTY;
}

void destroy_world(World * world)
{
	int i;
	for(i = 0; i < world->width_in_tiles; i++) {
		free(world->entities[i]);
	}
	free(world->entities);
}