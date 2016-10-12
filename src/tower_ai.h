// Bart Middag & Thomas Dhondt ; groepsnummer: 14 ; tower_ai.h

#ifndef	H_TOWER_AI
#define H_TOWER_AI

#include "entities.h"
#include "world.h"

int is_valid_target(Tower * t, Enemy * e);

Enemy * find_target(Tower * t, Entity * enemies, int enemies_length);

void shoot(Tower * t);

int is_out_of_range(Projectile * p, Tower * t, World * w);

void do_damage(Projectile * p, Enemy * e);

#endif