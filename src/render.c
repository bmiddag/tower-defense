// Bart Middag & Thomas Dhondt ; groepsnummer: 14 ; render.c

#include "gui.h"
#include "world.h"
#include "game.h"
#include "hud.h"
#include "render.h"
#include <stdio.h>

// zorgt voor de fade-out bij het game over scherm
int game_over_veil = 0;

void render_world_to_sprite(World * world) 
{
	int i, j;
	start_drawing_world();
	for (i = 0; i < world->width_in_tiles; i++) {
		for (j = 0; j < world->height_in_tiles; j++) {
			draw_sprite(SPRITE_GRASS, convert_tile2world_x(i), convert_tile2world_y(j), 0);
			render_entity(&world->entities[i][j]);
		}
	}
	stop_drawing_world();
}

void render_grid(World* world) {
	int i;
	Color color_black = {0, 0, 0, 128};

	for(i = 1; i < world->width_in_tiles; i++) {
		draw_line(color_black, convert_tile2screen_x(i), convert_tile2screen_y(0), convert_tile2screen_x(i), convert_tile2screen_y(SCREEN_HEIGHT), 2.0);
	}
	for(i = 1; i < world->height_in_tiles; i++) {
		draw_line(color_black, convert_tile2screen_x(0), convert_tile2screen_y(i), convert_tile2screen_x(SCREEN_WIDTH), convert_tile2screen_y(i), 2.0);
	}
}

void render_world(World * world)
{
	draw_sprite(SPRITE_WORLD, SCREEN_START_X, SCREEN_START_Y, 0);
	render_entity(world->spawn);
	render_entity(world->castle);

	if(SHOW_GRID) {
		render_grid(world);
	}
}

void render_spellscreen(Spells * spells)
{
	Color color_blue = {0, 0, 100, 5};
	Color color_green = {0, 100, 0, 5};

	if(spells->frost_wave_active){
		draw_rectangle(convert_tile2screen_x(0), convert_tile2screen_y(0), SCREEN_WIDTH, SCREEN_HEIGHT, color_blue);
	} else if (spells->poison_gas_active) {
		draw_rectangle(convert_tile2screen_x(0), convert_tile2screen_y(0), SCREEN_WIDTH, SCREEN_HEIGHT, color_green);
	}
}


void render_entity(Entity *entity)
{
	SPRITE_TYPE type;
	Color color_red = {255, 0, 0, 255};
	Color color_green = {0, 255, 0, 255};
	float x = convert_world2screen_x(entity->all.world_x);
	float y = convert_world2screen_y(entity->all.world_y);

	switch(entity->type) {
	case ENEMY:
		switch(entity->enemy.enemy_type) {
		case NORMAL: type = SPRITE_NORMAL; break;
		case ELITE: type = SPRITE_ELITE; break;
		case FAST: type = SPRITE_FAST; break;
		case AIR: type = SPRITE_AIR; break;
		case BOSS: type = SPRITE_BOSS; break;
		default: type = SPRITE_NORMAL; break;
		}
		draw_sprite_animated(type,&entity->enemy.frameAnimator,x,y,entity->enemy.angle);
		draw_line(color_red,x,y+1,x+32,y+1,2);
		draw_line(color_green,x,y+1,x+(32*entity->enemy.health/entity->enemy.health_max),y+1,2);
		break;
	case SPAWN_LOCATION:
		draw_sprite_animated(SPRITE_SPAWN,&entity->spawn_location.frameAnimator,x,y,0);
		break;
	case CASTLE:
		draw_sprite_animated(SPRITE_CASTLE,&entity->castle.frameAnimator,x,y,0);
		break;
	case TOWER:
		switch(entity->tower.tower_type) {
		case MACHINE_GUN: type = SPRITE_TOWER_MACHINE_GUN; break;
		case ROCKET_LAUNCHER: type = SPRITE_TOWER_ROCKET_LAUNCHER; break;
		case FLAK_CANNON: type = SPRITE_TOWER_FLAK_CANNON; break;
		default: type = SPRITE_TOWER_MACHINE_GUN; break;
		}
		draw_sprite(type,x,y,0);
		break;
	case OBSTACLE:
		switch(entity->obstacle.obstacle_type) {
		case WATER: type = SPRITE_WATER; break;
		case MOUNTAIN: type = SPRITE_MOUTAIN; break;
		default: type = SPRITE_WATER; break;
		}
		draw_sprite(type,convert_screen2world_x(x),convert_screen2world_y(y),0);
		break;
	}
}

void render_projectiles(Entity * tower)
{
	int i, amount;
	SPRITE_TYPE type;
	switch(tower->tower.tower_type) {
	case MACHINE_GUN:
		amount = MACHINE_GUN_AMMO;
		type = SPRITE_AMMO_BULLET;
		break;
	case ROCKET_LAUNCHER:
		amount = ROCKET_LAUNCHER_AMMO;
		type = SPRITE_AMMO_ROCKET;
		break;
	case FLAK_CANNON:
		amount = FLAK_CANNON_AMMO;
		type = SPRITE_AMMO_FLAK;
		break;
	default:
		amount = 0;
		type = SPRITE_AMMO_BULLET;
		break;
	}
	for(i = 0; i < amount; i++) {
		if(tower->tower.projectiles[i].live) {
			draw_sprite(type,convert_world2screen_x(tower->tower.projectiles[i].world_x),
				convert_world2screen_y(tower->tower.projectiles[i].world_y),tower->tower.projectiles[i].angle);
		}
	}
}

void render_ui(GameState * state)
{
	/* UI background */
	draw_sprite(SPRITE_UI, 0, 0, 0);

	/* Buttons */
	render_buttons(&state->hud);

	/* HUD */
	render_hud(&state->hud);
}

void render_hud(Hud * hud)
{	
	char money[20];
	char score[40];
	char wave[20];
	Color white = {255, 255, 255, 255};

	sprintf(money, "$%d", *hud->money);
	sprintf(score, "Score: %d - HP: %d - Mana: %d", *hud->score, *hud->castle_health, *hud->mana);
	sprintf(wave, "Wave %d", *hud->wave_number);

	draw_text(money, FONT_LARGE, white, 20, 6, ALIGN_LEFT);
	draw_text(score, FONT_LARGE, white, SCREEN_WIDTH/2, 6, ALIGN_CENTER);
	draw_text(wave, FONT_LARGE, white, SCREEN_WIDTH-20, 6, ALIGN_RIGHT);
}

void render_game_over(GameState * state)
{
	int i;
	Color color_black = {0, 0, 0, game_over_veil};
	Color color_white = {255, 255, 255, 255};
	if(state->game_over == 0) return;

	//Zet de animatie van de vijanden even stil, moet niet voor lang dankzij de fade-out.
	for(i = 0; i < state->enemies_length; i++) {
		state->enemies[i].enemy.frameAnimator.frameDelay = 1000;
	}

	draw_rectangle(convert_tile2screen_x(0), convert_tile2screen_y(0), SCREEN_WIDTH, SCREEN_HEIGHT, color_black);
	if(game_over_veil < 255) {
		//Game over scherm
		game_over_veil++;
		draw_text("GAME OVER", FONT_HUGE, color_white, SCREEN_START_X + (SCREEN_WIDTH - SCREEN_START_X) / 2, SCREEN_START_Y + (SCREEN_HEIGHT - SCREEN_START_Y) / 2 - 75/2, ALIGN_CENTER);
	} else {
		//Eindscherm
		draw_text("THANK YOU FOR PLAYING", FONT_LARGE, color_white, SCREEN_START_X + (SCREEN_WIDTH - SCREEN_START_X) / 2, SCREEN_START_Y + (SCREEN_HEIGHT - SCREEN_START_Y) / 5 - 18/2, ALIGN_CENTER);
		draw_text("TOWER DEFENSE", FONT_HUGE, color_white, SCREEN_START_X + (SCREEN_WIDTH - SCREEN_START_X) / 2, SCREEN_START_Y + (SCREEN_HEIGHT - SCREEN_START_Y) / 3.5 - 75/2, ALIGN_CENTER);
		draw_text("BY BART MIDDAG & THOMAS DHONDT", FONT_LARGE, color_white, SCREEN_START_X + (SCREEN_WIDTH - SCREEN_START_X) / 2, SCREEN_START_Y + (SCREEN_HEIGHT - SCREEN_START_Y) / 2.4 - 18/2, ALIGN_CENTER);
		draw_text("IN LOVING MEMORY OF", FONT_MEDIUM, color_white, SCREEN_START_X + (SCREEN_WIDTH - SCREEN_START_X) / 2, SCREEN_START_Y + (SCREEN_HEIGHT - SCREEN_START_Y) / 2 - 14/2, ALIGN_CENTER);
		for(i = 0; i < 5; i++) {
			switch(i) {
			case 0:
				draw_text("NORMAL", FONT_MEDIUM, color_white, SCREEN_START_X + (i+1)*(SCREEN_WIDTH - SCREEN_START_X) / 6, SCREEN_START_Y + (SCREEN_HEIGHT - SCREEN_START_Y) / 1.4 - 14/2, ALIGN_CENTER);
				draw_sprite(SPRITE_NORMAL, SCREEN_START_X + (i+1)*(SCREEN_WIDTH - SCREEN_START_X) / 6 - TILE_SIZE/2, SCREEN_START_Y + (SCREEN_HEIGHT - SCREEN_START_Y) / 1.65 - TILE_SIZE/2, PI);
				break;
			case 1:
				draw_text("ELITE", FONT_MEDIUM, color_white, SCREEN_START_X + (i+1)*(SCREEN_WIDTH - SCREEN_START_X) / 6, SCREEN_START_Y + (SCREEN_HEIGHT - SCREEN_START_Y) / 1.4 - 14/2, ALIGN_CENTER);
				draw_sprite(SPRITE_ELITE, SCREEN_START_X + (i+1)*(SCREEN_WIDTH - SCREEN_START_X) / 6 - TILE_SIZE/2, SCREEN_START_Y + (SCREEN_HEIGHT - SCREEN_START_Y) / 1.65 - TILE_SIZE/2, PI);
				break;
			case 2:
				draw_text("FAST", FONT_MEDIUM, color_white, SCREEN_START_X + (i+1)*(SCREEN_WIDTH - SCREEN_START_X) / 6, SCREEN_START_Y + (SCREEN_HEIGHT - SCREEN_START_Y) / 1.4 - 14/2, ALIGN_CENTER);
				draw_sprite(SPRITE_FAST, SCREEN_START_X + (i+1)*(SCREEN_WIDTH - SCREEN_START_X) / 6 - TILE_SIZE/2, SCREEN_START_Y + (SCREEN_HEIGHT - SCREEN_START_Y) / 1.65 - TILE_SIZE/2, PI);
				break;
			case 3:
				draw_text("AIR", FONT_MEDIUM, color_white, SCREEN_START_X + (i+1)*(SCREEN_WIDTH - SCREEN_START_X) / 6, SCREEN_START_Y + (SCREEN_HEIGHT - SCREEN_START_Y) / 1.4 - 14/2, ALIGN_CENTER);
				draw_sprite(SPRITE_AIR, SCREEN_START_X + (i+1)*(SCREEN_WIDTH - SCREEN_START_X) / 6 - TILE_SIZE/2, SCREEN_START_Y + (SCREEN_HEIGHT - SCREEN_START_Y) / 1.65 - TILE_SIZE/2, PI);
				break;
			case 4:
				draw_text("BOSS", FONT_MEDIUM, color_white, SCREEN_START_X + (i+1)*(SCREEN_WIDTH - SCREEN_START_X) / 6, SCREEN_START_Y + (SCREEN_HEIGHT - SCREEN_START_Y) / 1.4 - 14/2, ALIGN_CENTER);
				draw_sprite(SPRITE_BOSS, SCREEN_START_X + (i+1)*(SCREEN_WIDTH - SCREEN_START_X) / 6 - TILE_SIZE/2, SCREEN_START_Y + (SCREEN_HEIGHT - SCREEN_START_Y) / 1.65 - TILE_SIZE/2, PI);
			}
		}
		draw_text("SEE YOU NEXT TIME!", FONT_LARGE, color_white, SCREEN_START_X + (SCREEN_WIDTH - SCREEN_START_X) / 2, SCREEN_START_Y + (SCREEN_HEIGHT - SCREEN_START_Y) / 1.2 - 18/2, ALIGN_CENTER);
	}
}

void render_mouse_actions(GameState * state)
{
	Color color_red = {255, 0, 0, 127};
	Color color_green = {0, 255, 0, 127};
	Color color_white = {255, 255, 255, 25};
	if(state->action != BUILD_TOWER || !in_world_screen(state->mouse.screen_x, state->mouse.screen_y)) return;
	
	set_transparency_on();
	if (state->blueprint.valid) {
		draw_rectangle(convert_tile2screen_x(state->mouse.tile_x), convert_tile2screen_y(state->mouse.tile_y), convert_tile2screen_x(state->mouse.tile_x + 1), convert_tile2screen_y(state->mouse.tile_y + 1), color_green);
		draw_circle(convert_tile2screen_x(state->mouse.tile_x) + TILE_SIZE/2, convert_tile2screen_y(state->mouse.tile_y) + TILE_SIZE/2, state->blueprint.entity.tower.range, color_white);
	} else {
		draw_rectangle(convert_tile2screen_x(state->mouse.tile_x), convert_tile2screen_y(state->mouse.tile_y), convert_tile2screen_x(state->mouse.tile_x + 1), convert_tile2screen_y(state->mouse.tile_y + 1), color_red);
	}
	set_transparency_off();
}



/* === Implemented methods, DO NOT CHANGE === */

void render_paths(GameState * state)
{
	Path * path;
	Color red = {255, 0, 0, 255};
	Color orange = {255, 192, 0, 255};
	int i,j, x = 0, y = 0, prev_x = 0, prev_y = 0;
	float scale = 0.2;

	for (i = 0; i < state->enemies_length; i++) {
		path = &state->enemies[i].enemy.path;
		for (j = 0; j < path->length; j++) {
			prev_x = x;
			prev_y = y;
			x = convert_tile2screen_x(path->nodes[j].tile_x) + 0.5*TILE_SIZE;
			y = convert_tile2screen_y(path->nodes[j].tile_y) + 0.5*TILE_SIZE;

			if (j != 0) {
				draw_line(red, prev_x, prev_y, x, y, 2.0);
			}
			if (j == 0 || j == (path->length - 1)) {
				draw_rectangle(x-(TILE_SIZE*scale),y-(TILE_SIZE*scale), x+(TILE_SIZE*scale), y+(TILE_SIZE*scale), red);
			} else if (j == path->current_node_index) {
				draw_triangle(x, y-TILE_SIZE*(scale+0.1), x-TILE_SIZE*(scale+0.1), y+TILE_SIZE*(scale+0.1), x+TILE_SIZE*(scale+0.1), y+TILE_SIZE*(scale+0.1), red);
				draw_triangle(x, y-TILE_SIZE*scale, x-TILE_SIZE*scale, y+TILE_SIZE*(scale+0.05), x+TILE_SIZE*scale, y+TILE_SIZE*(scale+0.05), orange);
			} else {
				draw_circle(x, y, scale*TILE_SIZE, red);
			}

		}
	}
}
