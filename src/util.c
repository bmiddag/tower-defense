// Bart Middag & Thomas Dhondt ; groepsnummer: 14 ; util.c
#include <math.h>
#include "config.h"
#include "util.h"

float convert_world2screen_x(float world_x)
{
	return world_x+SCREEN_START_X;
}

float convert_world2screen_y(float world_y)
{
	return world_y+SCREEN_START_Y;
}

float convert_tile2screen_x(float tile_x)
{
	return convert_world2screen_x(tile_x*TILE_SIZE);
}

float convert_tile2screen_y(float tile_y)
{
	return convert_world2screen_y(tile_y*TILE_SIZE);
}

float convert_tile2world_x(int tile_x)
{
	return tile_x*TILE_SIZE;
}

float convert_tile2world_y(int tile_y)
{
	return tile_y*TILE_SIZE;
}

float convert_screen2world_x(float screen_x)
{
	return screen_x-SCREEN_START_X;
}
float convert_screen2world_y(float screen_y)
{
	return screen_y-SCREEN_START_Y;
}

int convert_screen2tile_x(float screen_x)
{
	return (int)convert_screen2world_x(screen_x)/TILE_SIZE;
}

int convert_screen2tile_y(float screen_y)
{
	return (int)convert_screen2world_y(screen_y)/TILE_SIZE;
}

int convert_world2tile_x(float world_x)
{
	return (int)world_x/TILE_SIZE;
}

int convert_world2tile_y(float world_y)
{
	return (int)world_y/TILE_SIZE;
}


float euclidean_distance(float x1, float y1, float x2, float y2)
{
	return sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
}

float find_alpha(float horizontal_length, float vertical_length, float diagonal_length)
{
	//Wie deelt door nul is een snul, dus vangen we dit geval beter eerst op!
	if(vertical_length == 0) return PI/2;
	return acosf(((vertical_length*vertical_length)+(diagonal_length*diagonal_length)-(horizontal_length*horizontal_length))/(2*vertical_length*diagonal_length));
}

void dissolve_speed(float alpha_radians, float speed, float * horizontal, float * vertical)
{
	*horizontal = sin(alpha_radians)*speed;
	*vertical = -cos(alpha_radians)*speed;
}

void calc_direction(float from_x, float from_y, float to_x, float to_y, int * direction_x, int * direction_y)
{
	if(to_x > from_x) {
		*direction_x = 1;
	} else if(to_x < from_x) {
		*direction_x = -1;
	} else *direction_x = 0;
	if(to_y > from_y) {
		*direction_y = 1;
	} else if(to_y < from_y) {
		*direction_y = -1;
	} else *direction_y = 0;
}

float find_render_angle(float alpha_radians, int direction_x, int direction_y)
{
	float angle;
	angle = 2*PI;
	if(direction_y>=0) angle+=PI;
	if((direction_x>=0)==(direction_y>=0)) {
		angle-=alpha_radians;
	} else {
		angle+=alpha_radians;
	}
	return fmod((float)angle,(float)(2*PI));
}




/* === Already implemented, DO NOT CHANGE */

int in_world_screen(float screen_x, float screen_y)
{
	return (screen_x >= SCREEN_START_X && screen_x <= SCREEN_WIDTH &&
		screen_y >= SCREEN_START_Y && screen_y <= SCREEN_HEIGHT);
}