// Bart Middag & Thomas Dhondt ; groepsnummer: 14 ; entities.c

#include <stdlib.h>
#include "entities.h"
#include "config.h"


void init_entity(Entity * entity, EntityType type, int world_x, int world_y, int width, int height)
{
	entity->type = type;
	entity->all.width = width;
	entity->all.height = height;
	entity->all.type = type;
	entity->all.world_x = world_x;
	entity->all.world_y = world_y;
}

int is_colliding(Entity * e1, Entity * e2)
{
	return euclidean_distance(e1->all.world_x, e1->all.world_y, e2->all.world_x, e2->all.world_y) <= TILE_SIZE/2;
}

void init_tower_blueprint(Entity * entity, TowerType tower_type)
{
	init_entity(entity,TOWER,entity->all.world_x,entity->all.world_y,convert_tile2world_x(1),convert_tile2world_y(1));
	entity->tower.tower_type = tower_type;
	switch(tower_type) {
	case MACHINE_GUN: 
		entity->tower.cost = MACHINE_GUN_COST;
		entity->tower.range = MACHINE_GUN_RANGE;
		break;
	case ROCKET_LAUNCHER: 
		entity->tower.cost = ROCKET_LAUNCHER_COST;
		entity->tower.range = ROCKET_LAUNCHER_RANGE;
		break;
	case FLAK_CANNON: 
		entity->tower.cost = FLAK_CANNON_COST;
		entity->tower.range = FLAK_CANNON_RANGE;
		break;
	default:
		entity->tower.cost = 0;
		entity->tower.range = 0;
		break;
	}

}

void init_machine_gun(Entity * entity, float world_x, float world_y)
{
	int i;

	init_entity(entity,TOWER,world_x,world_y,convert_tile2world_x(1),convert_tile2world_y(1));
	entity->tower.world_x = world_x;
	entity->tower.world_y = world_y;
	entity->tower.tower_type = MACHINE_GUN;
	entity->tower.ammo = MACHINE_GUN_AMMO;
	entity->tower.cost = MACHINE_GUN_COST;
	entity->tower.frames_since_last_shot = 0;
	entity->tower.projectiles = (Projectile*) calloc(MACHINE_GUN_AMMO,sizeof(Projectile));

	for(i = 0; i < MACHINE_GUN_AMMO; i++) {
		entity->tower.projectiles[i].damage = BULLET_DAMAGE;
		entity->tower.projectiles[i].speed = BULLET_SPEED;
		entity->tower.projectiles[i].type = PROJECTILE;
		entity->tower.projectiles[i].world_x = entity->tower.world_x;
		entity->tower.projectiles[i].world_y = entity->tower.world_y;
		entity->tower.projectiles[i].projectile_type = BULLET;
		entity->tower.projectiles[i].width = BULLET_SIZE;
		entity->tower.projectiles[i].height = BULLET_SIZE;
	}

	entity->tower.shoot_interval = MACHINE_GUN_SHOOT_INTERVAL;
	entity->tower.range = MACHINE_GUN_RANGE;
}

void init_rocket_launcher(Entity * entity, float world_x, float world_y)
{
	int i;

	init_entity(entity,TOWER,world_x,world_y,convert_tile2world_x(1),convert_tile2world_y(1));
	entity->tower.world_x = world_x;
	entity->tower.world_y = world_y;
	entity->tower.tower_type = ROCKET_LAUNCHER;
	entity->tower.ammo = ROCKET_LAUNCHER_AMMO;
	entity->tower.cost = ROCKET_LAUNCHER_COST;
	entity->tower.frames_since_last_shot = 0;
	entity->tower.projectiles = (Projectile*) calloc(ROCKET_LAUNCHER_AMMO,sizeof(Projectile));

	for(i = 0; i < ROCKET_LAUNCHER_AMMO; i++) {
		entity->tower.projectiles[i].damage = ROCKET_DAMAGE;
		entity->tower.projectiles[i].speed = ROCKET_SPEED;
		entity->tower.projectiles[i].type = PROJECTILE;
		entity->tower.projectiles[i].world_x = entity->tower.world_x;
		entity->tower.projectiles[i].world_y = entity->tower.world_y; 
		entity->tower.projectiles[i].projectile_type = ROCKET;
		entity->tower.projectiles[i].width = ROCKET_SIZE;
		entity->tower.projectiles[i].height = ROCKET_SIZE;
	}

	entity->tower.shoot_interval = ROCKET_LAUNCHER_INTERVAL;
	entity->tower.range = ROCKET_LAUNCHER_RANGE;
}

void init_flak_cannon(Entity * entity, float world_x, float world_y)
{
	int i;

	init_entity(entity,TOWER,world_x,world_y,convert_tile2world_x(1),convert_tile2world_y(1));
	entity->tower.world_x = world_x;
	entity->tower.world_y = world_y;
	entity->tower.tower_type = FLAK_CANNON;
	entity->tower.ammo = FLAK_CANNON_AMMO;
	entity->tower.cost = FLAK_CANNON_COST;
	entity->tower.frames_since_last_shot = 0;
	entity->tower.projectiles = (Projectile*) calloc(FLAK_CANNON_AMMO,sizeof(Projectile));

	for(i = 0; i < FLAK_CANNON_AMMO; i++) {
		entity->tower.projectiles[i].damage = FLAK_DAMAGE;
		entity->tower.projectiles[i].speed = FLAK_SPEED;
		entity->tower.projectiles[i].type = PROJECTILE;
		entity->tower.projectiles[i].world_x = entity->tower.world_x;
		entity->tower.projectiles[i].world_y = entity->tower.world_y; 
		entity->tower.projectiles[i].projectile_type = FLAK;
		entity->tower.projectiles[i].width = FLAK_SIZE;
		entity->tower.projectiles[i].height = FLAK_SIZE;
	}

	entity->tower.shoot_interval = FLAK_CANNON_INTERVAL;
	entity->tower.range = FLAK_CANNON_RANGE;
}

void init_enemy(Entity * enemy, EnemyType type)
{
	switch(type) {
	case NORMAL:
		enemy->enemy.damage = ENEMY_NORMAL_DAMAGE;
		enemy->enemy.health_max = ENEMY_NORMAL_HEALTH;
		enemy->enemy.speed = ENEMY_NORMAL_SPEED;
		init_frameAnimator(&enemy->enemy.frameAnimator, 2, 4, 5, TILE_SIZE, TILE_SIZE);
		break;
	case ELITE:
		enemy->enemy.damage = ENEMY_ELITE_DAMAGE;
		enemy->enemy.health_max = ENEMY_ELITE_HEALTH;
		enemy->enemy.speed = ENEMY_ELITE_SPEED;
		init_frameAnimator(&enemy->enemy.frameAnimator, 3, 4, 5, TILE_SIZE, TILE_SIZE);
		break;
	case FAST:
		enemy->enemy.damage = ENEMY_FAST_DAMAGE;
		enemy->enemy.health_max = ENEMY_FAST_HEALTH;
		enemy->enemy.speed = ENEMY_FAST_SPEED;
		init_frameAnimator(&enemy->enemy.frameAnimator, 2, 4, 10, TILE_SIZE, TILE_SIZE);
		break;
	case AIR:
		enemy->enemy.damage = ENEMY_AIR_DAMAGE;
		enemy->enemy.health_max = ENEMY_AIR_HEALTH;
		enemy->enemy.speed = ENEMY_AIR_SPEED;
		init_frameAnimator(&enemy->enemy.frameAnimator, 2, 4, 5, TILE_SIZE, TILE_SIZE);
		break;
	case BOSS:
		enemy->enemy.damage = ENEMY_BOSS_DAMAGE;
		enemy->enemy.health_max = ENEMY_BOSS_HEALTH;
		enemy->enemy.speed = ENEMY_BOSS_SPEED;
		init_frameAnimator(&enemy->enemy.frameAnimator, 3, 4, 5, TILE_SIZE, TILE_SIZE);
		break;
	}
	enemy->enemy.health = enemy->enemy.health_max;
	enemy->enemy.enemy_type = type;
	enemy->enemy.alive = 1;
}

/* === Implemented functions, DO NOT CHANGE! */

void init_frameAnimator(FrameAnimator * animator, int animationColumns, int maxFrame, int frameDelay, int frameHeight, int frameWidth)
{
	animator->animationColumns = animationColumns;
	animator->animationDirection = 1;
	animator->curFrame = 0;
	animator->frameCounter = 0;
	animator->frameDelay = frameDelay;
	animator->frameHeight = frameHeight;
	animator->frameWidth =  frameWidth;
	animator->maxFrame = maxFrame;
}