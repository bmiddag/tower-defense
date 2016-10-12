// Bart Middag & Thomas Dhondt ; groepsnummer: 14 ; tower_ai.c
#include "entities.h"
#include "tower_ai.h"
#include "world.h"
#include "util.h"
#include <stdlib.h>
#include <stdio.h>
#include "config.h"
#include <math.h>

int is_valid_target(Tower * t, Enemy * e)
{
	if(!e->alive) return 0;
	//Can we shoot it please?
	if(t->tower_type == FLAK_CANNON) {
		return (e->enemy_type == AIR && euclidean_distance(t->world_x, t->world_y, e->world_x, e->world_y) <= t->range);
	} else {
		return (e->enemy_type != AIR && euclidean_distance(t->world_x, t->world_y, e->world_x, e->world_y) <= t->range);
	}
}

Enemy * find_target(Tower * t, Entity * enemies, int enemies_length)
{
	//Vind het eerste target waarop we kunnen schieten!
	int i;
	for(i = 0; i < enemies_length; i++){
		if(is_valid_target(t,&enemies[i].enemy)){
			return &enemies[i].enemy;
		}
	}
	return NULL;
}

void shoot(Tower * t)
{
	int i;
	float alpha, dist;
	for(i = 0; i < t->ammo; i++) {
		if(!t->projectiles[i].live) break;
	}
	if((i >= t->ammo)||(t->target == NULL)||(t->frames_since_last_shot<t->shoot_interval)){
		return;
	}

	//Stel het projectile correct in.
	t->frames_since_last_shot = 0;
	t->projectiles[i].live = 1;
	t->projectiles[i].world_x = t->world_x;
	t->projectiles[i].world_y = t->world_y;
	t->projectiles[i].target = t->target;

	calc_direction(t->world_x,t->world_y,t->target->world_x,t->target->world_y,
		&t->projectiles[i].direction_x,&t->projectiles[i].direction_y);

	dist = euclidean_distance(t->world_x,t->world_y,t->target->world_x,t->target->world_y);
	alpha = find_alpha(fabs(t->world_x-t->target->world_x),fabs(t->world_y-t->target->world_y),dist);
	t->projectiles[i].angle = find_render_angle(alpha,t->projectiles[i].direction_x,t->projectiles[i].direction_y);
}

int is_out_of_range(Projectile * p, Tower * t, World * w)
{
	return ((euclidean_distance(t->world_x,t->world_y,p->world_x,p->world_y) > t->range) || (p->world_x < 0) || (p->world_y < 0)
		|| (convert_world2tile_x(p->world_x) > w->width_in_tiles) || (convert_world2tile_y(p->world_y) > w->height_in_tiles ));
}

void do_damage(Projectile * p, Enemy * e)
{
	if(((p->projectile_type == FLAK) && (e->enemy_type == AIR)) || ((p->projectile_type != FLAK) && (e->enemy_type != AIR))) {
		e->health -= p->damage;
	}
}