// Bart Middag & Thomas Dhondt ; groepsnummer: 14 ; game.c

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "config.h"
#include "world.h"
#include "gui.h"
#include "render.h"
#include "pathfinding.h"
#include "entities.h"
#include "util.h"
#include "game.h"
#include "tower_ai.h"
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>

/* === Prototypes === */
void set_button_hover_state(GameState * state);
void set_button_action(GameState * state);
void reset_button_action(GameState * state);
void reset_all_buttons_action(GameState * state);
void check_blueprint(GameState * state);
void destroy_enemy(GameState * state, int index);
void spawn_enemies(GameState * state);
void check_enemy_wave(GameState * state);
int spawn_special(float percent);

//Extra's (muziek)
void add_music_volume(float game, float back, float drum);
ALLEGRO_SAMPLE *music_game_sample = NULL;
ALLEGRO_SAMPLE *music_back_sample = NULL;
ALLEGRO_SAMPLE *music_drum_sample = NULL;
ALLEGRO_SAMPLE *sfx_game_over = NULL;
ALLEGRO_SAMPLE_INSTANCE *music_game = NULL;
ALLEGRO_SAMPLE_INSTANCE *music_back = NULL;
ALLEGRO_SAMPLE_INSTANCE *music_drum = NULL;
float music_game_volume = 1.0;
float music_back_volume = 1.0;
float music_drum_volume = 0.0;

int run_game_loop(GameState * state)
{
	int done = 0;
	int i = 0;
	
	/* Initialise gui */
	init_gui(SCREEN_WIDTH,SCREEN_HEIGHT);

	/* Load resources */
	init_sprite_cache();
	render_world_to_sprite(&state->world);
	update_hud(
		&state->hud, 
		&state->score,
		&state->mana,
		&state->money, 
		&state->world.castle->castle.health,
		&state->wave.wave_number);

	/* Initialise game loop */
	init_game_loop(FPS);
	while (!done)
	{
		/* Get events */
		Event ev;
		wait_for_event(&ev);

		/* Event handlers */
		switch (ev.type) {
			case EVENT_TIMER:
				state->redraw = 1;
				if(!state->game_over) {
					check_spells(state);
					check_enemy_wave(state);
					update_movement(state);
					do_tower_attacks(state);
				}
				break;
			case EVENT_MOUSE_MOVE:
				mouse_move(&ev.mouseMoveEvent, state);
				break;
			case EVENT_MOUSE_DOWN:
				mouse_down(&ev.mouseDownEvent, state);
				break;
			case EVENT_MOUSE_UP:
				mouse_up(&ev.mouseUpEvent, state);
				break;
			case EVENT_DISPLAY_CLOSE:
				done = 1;
				break;
		}	

		/* Render only on timer event AND if all movement and logic was processed */
		if (state->redraw && all_events_processed()) { 
			render_game(state);
		}
	}

	/* Cleanup */
	cleanup_game_loop();
	cleanup_sprite_cache();
	return 0;
}

void add_music_volume(float game, float back, float drum) {
	music_game_volume+=game;
	music_back_volume+=back;
	music_drum_volume+=drum;
	if(music_game_volume>1.0) music_game_volume = 1.0;
	if(music_game_volume<0.0) music_game_volume = 0.0;
	if(music_back_volume>1.0) music_back_volume = 1.0;
	if(music_back_volume<0.0) music_back_volume = 0.0;
	if(music_drum_volume>1.0) music_drum_volume = 1.0;
	if(music_drum_volume<0.0) music_drum_volume = 0.0;
	if(al_is_audio_installed() && music_game != NULL) al_set_sample_instance_gain(music_game,music_game_volume);
	if(al_is_audio_installed() && music_back != NULL) al_set_sample_instance_gain(music_back,music_back_volume);
	if(al_is_audio_installed() && music_drum != NULL) al_set_sample_instance_gain(music_drum,music_drum_volume);
}

void check_spells(GameState * state) {
	if(state->mana<MANA_MAX) state->mana++;
	if(state->spells.frost_wave_active || state->spells.poison_gas_active) {
		//Laat de melodie wegvallen tijdens spells
		add_music_volume(-0.01,0.0,0.0);
		if(state->action == GENERATE_FROST_WAVE || state->action == RELEASE_POISON_GAS) state->action = NONE;
		state->spells.spell_duration++;
		if((state->spells.frost_wave_active) && (state->spells.spell_duration >= FROST_WAVE_DURATION)) {
			state->spells.frost_wave_active = 0;
		} else if((state->spells.poison_gas_active) && (state->spells.spell_duration >= POISON_GAS_DURATION)) {
			state->spells.poison_gas_active = 0;
		}
	} else {
		//Laat de melodie spelen als er geen spells actief zijn
		add_music_volume(0.01,0.0,0.0);
		state->spells.spell_duration = 0;
		if(state->action == GENERATE_FROST_WAVE) {
			if(state->mana >= FROST_WAVE_COST) {
				state->spells.frost_wave_active = 1;
				state->mana-=FROST_WAVE_COST;
			}
			state->action = NONE;
		} else if(state->action == RELEASE_POISON_GAS) {
			if(state->mana >= POISON_GAS_COST) {
				state->spells.poison_gas_active = 1;
				state->mana-=POISON_GAS_COST;
			}
			state->action = NONE;
		}
	}
}

//Moet er een speciale enemy gespawnd worden?
int spawn_special(float percent){
	int n;
	n = rand() % 100;
	if(n < percent*100){
		return 1;
	} else{
		return 0;
	}
}

void spawn_enemies(GameState * state){
	int air, i;
	air = 0;
	if(!state->spells.frost_wave_active) state->wave.spawn_counter++;
	if(state->wave.spawn_counter == state->wave.spawn_interval && state->wave.nr_of_spawned_enemies < state->wave.nr_of_enemies){
		int index;
		index = state->wave.nr_of_spawned_enemies;
		state->wave.nr_of_spawned_enemies++;
		if(state->wave.boss_wave && state->wave.nr_of_spawned_enemies == state->wave.nr_of_enemies){
			//spawn boss
			init_entity(&state->enemies[index], ENEMY, state->world.spawn->all.world_x, state->world.spawn->all.world_y, TILE_SIZE, TILE_SIZE);
			init_enemy(&state->enemies[index], BOSS);
		}
		else if(state->wave.wave_number >= 8 && spawn_special(FAST_SPAWN_RATIO)){
			//spawn fast
			init_entity(&state->enemies[index], ENEMY, state->world.spawn->all.world_x, state->world.spawn->all.world_y, TILE_SIZE, TILE_SIZE);
			init_enemy(&state->enemies[index], FAST);
		}
		else if(state->wave.wave_number >= 4 && spawn_special(AIR_SPAWN_RATIO)){
			//spawn air
			air = 1;
			init_entity(&state->enemies[index], ENEMY, state->world.spawn->all.world_x, state->world.spawn->all.world_y, TILE_SIZE, TILE_SIZE);
			init_enemy(&state->enemies[index], AIR);
		}
		else if(state->wave.nr_of_spawned_enemies % 5 == 0){
			//spawn boss
			init_entity(&state->enemies[index], ENEMY, state->world.spawn->all.world_x, state->world.spawn->all.world_y, TILE_SIZE, TILE_SIZE);
			init_enemy(&state->enemies[index], ELITE);
		}
		else{
			// spawn normal
			init_entity(&state->enemies[index], ENEMY, state->world.spawn->all.world_x, state->world.spawn->all.world_y, TILE_SIZE, TILE_SIZE);
			init_enemy(&state->enemies[index], NORMAL);
		}
		state->enemies[index].enemy.health_max = state->enemies[index].enemy.health_max + (state->wave.wave_number * (state->wave.wave_number/3) * state->enemies[index].enemy.health_max * 0.1);
		state->enemies[index].enemy.health = state->enemies[index].enemy.health_max;
		if(! air ) {
			if (create_path(&state->enemies[index].enemy.path, &state->world, state->world.spawn, state->world.castle, OVER_LAND)) {
				state->enemies[index].enemy.direction_x = state->enemies[index].enemy.path.nodes[1].tile_x - state->enemies[index].enemy.path.nodes[0].tile_x;
				state->enemies[index].enemy.direction_y = state->enemies[index].enemy.path.nodes[1].tile_y - state->enemies[index].enemy.path.nodes[0].tile_y;
				state->enemies_length++;
			}
		} else {
			if (create_path(&state->enemies[index].enemy.path, &state->world, state->world.spawn, state->world.castle, OVER_AIR)) {
				state->enemies[index].enemy.direction_x = state->enemies[index].enemy.path.nodes[1].tile_x - state->enemies[index].enemy.path.nodes[0].tile_x;
				state->enemies[index].enemy.direction_y = state->enemies[index].enemy.path.nodes[1].tile_y - state->enemies[index].enemy.path.nodes[0].tile_y;
				state->enemies_length++;
			}
		}
		state->wave.spawn_counter = 0;
		spawn_enemies(state);
	} else if (state->wave.nr_of_spawned_enemies >= state->wave.nr_of_enemies){
		for(i = 0; i < state->enemies_length; i++) {
			if(state->enemies[i].enemy.alive) break;
		}
		if(state->enemies_length == i){
			state->wave.spawn_counter = state->wave.spawn_interval - 1;
			state->wave.nr_of_spawned_enemies = 0;
			state->enemies_length = 0;
			state->wave.started = 0;
			state->wave.completed = 1;
		}
	}

}

void check_enemy_wave(GameState * state)
{
	if(state->wave.cool_down_counter < state->wave.cool_down){
		state->wave.cool_down_counter++;
		add_music_volume(0.0,0.0,-0.01);
	}
	
	if(state->wave.cool_down_counter == state->wave.cool_down){
		add_music_volume(0.0,0.0,0.01);
		if(!state->wave.started){
			state->wave.boss_wave = (state->wave.wave_number % 5 == 0);
			state->wave.nr_of_enemies = 4 + state->wave.wave_number;
			state->wave.started = 1;
			state->enemies = (Entity *) realloc (state->enemies,state->wave.nr_of_enemies*sizeof(Entity));
		}
		spawn_enemies(state);
		if(state->wave.completed){
			state->wave.wave_number++;
			state->wave.cool_down_counter = 0;
			state->wave.completed = 0;
		}
	}
}

void destroy_enemy(GameState * state, int index) {
	Entity to_destroy;
	if(!state->enemies[index].enemy.alive) return;
	to_destroy = state->enemies[index];
	destroy_path(&state->enemies[index].enemy.path);
	state->enemies[index].enemy.alive = 0;
}

void update_movement(GameState * state) {
	int i, j, next, amount;
	float dist, alpha, speed_x, speed_y;
	Enemy *current_enemy;
	Projectile *proj;
	//Update movement voor enemies
	for (i = 0; i < state->enemies_length; i++) {
		if(!state->enemies[i].enemy.alive) continue;
		if(state->refresh_paths) refresh_path(&state->enemies[i].enemy.path, &state->world);
		current_enemy = &state->enemies[i].enemy;
		next = current_enemy->path.current_node_index;
		if (current_enemy->world_x == convert_tile2world_x(current_enemy->path.nodes[next].tile_x)
			&& current_enemy->world_y == convert_tile2world_y(current_enemy->path.nodes[next].tile_y)) {
			next++;
			if (next < current_enemy->path.length) {
				//Controleer blueprint bij verzetting van current_node_index
				current_enemy->path.current_node_index = next;
				check_blueprint(state);
			}
		}
		if (next < current_enemy->path.length) {
			calc_direction(current_enemy->world_x,current_enemy->world_y,convert_tile2world_x(current_enemy->path.nodes[next].tile_x),
				convert_tile2world_x(current_enemy->path.nodes[next].tile_y),&current_enemy->direction_x,&current_enemy->direction_y);
			current_enemy->angle = find_render_angle(fabs((PI/2)*current_enemy->direction_x),current_enemy->direction_x,current_enemy->direction_y);
			if(!state->spells.frost_wave_active) {
				//Zet de animatie terug goed
				if(current_enemy->frameAnimator.frameDelay > FROST_WAVE_DURATION) {
					switch(current_enemy->enemy_type) {
					case FAST: current_enemy->frameAnimator.frameDelay = 10; break;
					default: current_enemy->frameAnimator.frameDelay = 5; break;
					}
				}
				//Doe poison damage
				if(state->spells.poison_gas_active) {
					if(state->spells.spell_duration % 20 == 0) current_enemy->health-=(current_enemy->health_max*15/POISON_GAS_DURATION);
				}
				current_enemy->world_x+=current_enemy->direction_x*current_enemy->speed;
				current_enemy->world_y+=current_enemy->direction_y*current_enemy->speed;
			} else {
				//Bevries de animatie
				current_enemy->frameAnimator.frameDelay = FROST_WAVE_DURATION+40;
			}
			if(current_enemy->health <= 0) {
				state->money+= INCOME_FEE;
				state->score += current_enemy->health_max;
				//state->score += current_enemy->health_max + state->wave.wave_number*current_enemy->health_max/10;
				destroy_enemy(state,i);
			}
		} else {
			//De enemy heeft het kasteel bereikt!
			state->world.castle->castle.health-=current_enemy->damage;
			if(state->world.castle->castle.health <= 0) {
				//Game over!
				state->world.castle->castle.health = 0;
				state->game_over = 1;
				al_play_sample(sfx_game_over, 1.0, 0.0,1.0,ALLEGRO_PLAYMODE_ONCE,NULL);
				add_music_volume(-1.0,1.0,-1.0);
			}
			destroy_enemy(state,i);
		}
	}
	state->refresh_paths = 0;
	//Beweeg de projectiles voor elke toren
	for(i = 0; i < state->towers_length; i++) {
		switch(state->towers[i]->tower.tower_type) {
		case MACHINE_GUN: amount = MACHINE_GUN_AMMO; break;
		case ROCKET_LAUNCHER: amount = ROCKET_LAUNCHER_AMMO; break;
		case FLAK_CANNON: amount = FLAK_CANNON_AMMO; break;
		default: amount = 0; break;
		}
		for(j = 0; j < amount; j++) {
			proj = &state->towers[i]->tower.projectiles[j];
			if(proj->live) {
				if(is_out_of_range(proj, &state->towers[i]->tower, &state->world)) {
					proj->live = 0;
					continue;
				}
				if(proj->target->alive){
					if(is_colliding((Entity*)proj,(Entity*)proj->target)){
						do_damage(proj,proj->target);
						proj->live=0;
						continue;
					}
					calc_direction(proj->world_x,proj->world_y,proj->target->world_x,proj->target->world_y,&proj->direction_x,&proj->direction_y);
					dist = euclidean_distance(proj->world_x,proj->world_y,proj->target->world_x,proj->target->world_y);
					alpha = find_alpha(fabs(proj->world_x-proj->target->world_x),fabs(proj->world_y-proj->target->world_y),dist);
					proj->angle = find_render_angle(alpha,proj->direction_x,proj->direction_y);
				}
				dissolve_speed(proj->angle,proj->speed,&speed_x,&speed_y);
				proj->world_x += speed_x;
				proj->world_y += speed_y;
			}
		}
	}
}

void do_tower_attacks(GameState * state)
{
	int i;
	//Schiet, torens, schiet!
	for(i = 0 ; i < state->towers_length ; i++){
		state->towers[i]->tower.frames_since_last_shot++;
		state->towers[i]->tower.target = find_target(&state->towers[i]->tower,state->enemies,state->enemies_length);
		shoot(&state->towers[i]->tower);
	}
}

void init_game_state(GameState * state)
{
	init_world_from_file(&state->world, "assets/worlds/world1.world");
	state->world.castle->castle.health = CASTLE_HEALTH;
	//state->world.castle->castle.health = 2;
	state->action = NONE;
	state->game_over = 0;
	state->money = 250;
	state->score = 0;
	//state->mana = 5000;
	state->mana = 1200;
	state->towers = (Entity**) malloc (sizeof(Entity*));
	state->towers_length = 0;
	state->enemies_length = 0;
	//state->wave.wave_number = 25;
	state->wave.wave_number = 1;
	state->wave.nr_of_spawned_enemies = 0;
	state->wave.started = 0;
	state->wave.completed = 0;
	state->wave.cool_down = 6*FPS;
	state->wave.cool_down_counter = 0;
	state->wave.boss_wave = 0;
	state->wave.spawn_interval = 4*FPS;
	state->wave.spawn_counter = state->wave.spawn_interval-1;
	state->wave.nr_of_enemies = 4;
	state->spells.frost_wave_active = 0;
	state->spells.poison_gas_active = 0;
	state->spells.spell_duration = FROST_WAVE_DURATION;
	state->enemies = (Entity *) malloc (sizeof(Entity));
	/* Initialise music */
	do {
		//Initialiseer Allegro eerder om de muziek hier te kunnen inladen - klein trucje!
		if(!al_init()) break;
		//Maak de buffer groot genoeg, zodat de muziek niet van de minste lag blijft hangen.
		al_set_config_value(al_get_system_config(), "directsound", "buffer_size", "8192");
		if(!al_install_audio()) break;
		if(!al_init_acodec_addon()) break;
		if (!al_reserve_samples(4)) break;
		//Laad de muziek in.
		music_game_sample = al_load_sample("assets/audio/TD_GAME.ogg");
		music_back_sample = al_load_sample("assets/audio/TD_BACK.ogg");
		music_drum_sample = al_load_sample("assets/audio/TD_DRUM.ogg");
		sfx_game_over = al_load_sample("assets/audio/TD_GAMEOVER.ogg");
		music_game = al_create_sample_instance(music_game_sample);
		music_back = al_create_sample_instance(music_back_sample);
		music_drum = al_create_sample_instance(music_drum_sample);
		al_attach_sample_instance_to_mixer(music_game,al_get_default_mixer());
		al_attach_sample_instance_to_mixer(music_back,al_get_default_mixer());
		al_attach_sample_instance_to_mixer(music_drum,al_get_default_mixer());
		al_set_sample_instance_playmode(music_game,ALLEGRO_PLAYMODE_LOOP);
		al_set_sample_instance_playmode(music_back,ALLEGRO_PLAYMODE_LOOP);
		al_set_sample_instance_playmode(music_drum,ALLEGRO_PLAYMODE_LOOP);
		al_set_sample_instance_gain(music_drum,0.0);
		al_play_sample_instance(music_game);
		al_play_sample_instance(music_back);
		al_play_sample_instance(music_drum);
	} while(0);

	/* Initialise Buttons */
	init_buttons(&state->hud);
}

void destroy_game_state(GameState * state)
{
	int i;
	for (i = 0; i < state->enemies_length; i++) {
		destroy_enemy(state,i);
	}
	free(state->enemies);
	for(i = 0; i < state->towers_length; i++) {
		destroy_tower(&state->world,state->towers[i]);
	}
	destroy_world(&state->world);
	free(state->towers);
	//Free de muziek
	if(al_is_audio_installed()) {
		al_stop_samples();
		if(music_game_sample!=NULL)	al_destroy_sample(music_game_sample);
		if(music_back_sample!=NULL)	al_destroy_sample(music_back_sample);
		if(music_drum_sample!=NULL)	al_destroy_sample(music_drum_sample);
		if(sfx_game_over!=NULL) al_destroy_sample(sfx_game_over);
	}
}

void render_game(GameState * state)
{
	/* Variables */
	Color color = {255, 0, 255, 255};
	Color black = {0, 0, 0, 255};
	Color white = {255, 255, 255, 255};
	char buffer [20];
	int i,n;

	/* Set redraw off */
	state->redraw = 0;

	/* Render world */
	render_world(&state->world);

	/* Render pathfinding, if enabled */
	if (SHOW_PATHFINDING)
		render_paths(state);
	
	/* Render towers */
	for(i = 0; i < state->towers_length; i++) {
		render_entity(state->towers[i]);
	}
	
	/* Render enemies if they are alive */
	for(i = 0; i < state->enemies_length; i++) {
		if(state->enemies[i].enemy.alive)
			render_entity(&state->enemies[i]);
	}

	/* Render spellscreen */
	render_spellscreen(&state->spells);

	/* Render projectiles */
	for(i = 0; i < state->towers_length; i++) {
		render_projectiles(state->towers[i]);
	}

	
	/* render selection */
	render_mouse_actions(state);

	/* Render ui on top */
	render_ui(state);

	/* Fps rendering */
	n = sprintf(buffer, "%#.1f FPS", get_current_fps());
	draw_text(buffer, FONT_LARGE, color, SCREEN_WIDTH-20 -100, 6, ALIGN_RIGHT);

	/* Game over? */
	render_game_over(state);

	/* Render to screen */
	flip_display();
	clear_to_color(color);
}

void update_mouse(GameState * state, float screen_x, float screen_y)
{
	Mouse * mouse = &state->mouse;
	int prev_tile_x = mouse->tile_x;
	int prev_tile_y = mouse->tile_y;
	
	mouse->screen_x = screen_x;
	mouse->screen_y = screen_y;
	mouse->world_x = convert_screen2world_x(screen_x);
	mouse->world_y = convert_screen2world_y(screen_y);
	if (in_world_screen(mouse->screen_x, mouse->screen_y)) {
		mouse->tile_x = convert_screen2tile_x(screen_x);
		mouse->tile_y = convert_screen2tile_y(screen_y);
	}
	
	mouse->tile_changed |= (mouse->tile_x != prev_tile_x) || (mouse->tile_y != prev_tile_y);
}

void check_blueprint(GameState *state) {
	Mouse * mouse = &state->mouse;
	int i;
	if(state->game_over) state->action = NONE;
	if((!in_world_screen(state->mouse.screen_x, state->mouse.screen_y)) || (state->action != BUILD_TOWER)) return;
	if(state->money < state->blueprint.entity.tower.cost) {
		state->blueprint.valid = 0; state->action = NONE; return;
	}
	if (state->world.entities[mouse->tile_x][mouse->tile_y].type == EMPTY) {
		Path path;
		TrajectoryType type;
		int x, y, end_x, end_y;
		state->world.entities[mouse->tile_x][mouse->tile_y].type = TOWER;
		x = convert_world2tile_x(state->world.spawn->spawn_location.world_x);
		y = convert_world2tile_y(state->world.spawn->spawn_location.world_y);
		end_x = convert_world2tile_x(state->world.castle->all.world_x);
		end_y = convert_world2tile_y(state->world.castle->all.world_y);
		path.nodes = NULL;
		type = OVER_LAND;
		if(create_path(&path,&state->world,&state->world.entities[x][y],&state->world.entities[end_x][end_y],type)) {
			for (i = 0; i < state->enemies_length; i++) {
				if(!state->enemies[i].enemy.alive) continue;
				destroy_path(&path);
				x = state->enemies[i].enemy.path.nodes[state->enemies[i].enemy.path.current_node_index].tile_x;
				y = state->enemies[i].enemy.path.nodes[state->enemies[i].enemy.path.current_node_index].tile_y;
				type = state->enemies[i].enemy.path.trajectory_type;
				if(create_path(&path,&state->world,&state->world.entities[x][y],&state->world.entities[end_x][end_y],type) == 0) break;
			}
			state->blueprint.valid = (i==state->enemies_length);
		} else {
			state->blueprint.valid = 0;
		}
		destroy_path(&path);
		state->world.entities[mouse->tile_x][mouse->tile_y].type = EMPTY;
	} else {
		state->blueprint.valid = 0;
	}
}

/* === Event handlers === */

void mouse_move(MouseMoveEvent * ev, GameState * state)
{	
	/* Update mouse pointer location */
	update_mouse(state, ev->screen_x, ev->screen_y);

	if (in_world_screen(state->mouse.screen_x, state->mouse.screen_y)) {
		/* IMPLEMENT HERE*/
		if (state->action == BUILD_TOWER && state->mouse.tile_changed) {
			state->mouse.tile_changed = 0;
			check_blueprint(state);
			state->blueprint.entity.all.world_x = state->mouse.world_x;
			state->blueprint.entity.all.world_y = state->mouse.world_y;
		}
		/* Already implemented code to reset buttons after hovering. DO NOT CHANGE */
		reset_all_buttons_action(state);
	}
	
	else { /* Outside of world frame => buttons */
		set_button_hover_state(state); /* DO NOT CHANGE */
	}
}

void mouse_down(MouseDownEvent * ev, GameState * state)
{
	if (ev->button == 1) {
		Mouse * mouse = &state->mouse;
		int i;

		if (in_world_screen(mouse->screen_x, mouse->screen_y)) {
			//We mogen geen torens zetten of vernietigen als het spel gedaan is.
			if(state->game_over) {
				state->action = NONE;
				return;
			}

			//Toren bouwen of vernietigen?
			switch(state->action) {
			case BUILD_TOWER:
				if (state->blueprint.valid) {
					state->money-=state->blueprint.entity.tower.cost;
					state->blueprint.valid = 0;
					//Maak de toren aan.
					state->towers = (Entity**) realloc(state->towers,sizeof(Entity*)*(state->towers_length+1));
					state->towers[state->towers_length] = place_tower(&state->world,state->blueprint.entity.tower.tower_type,convert_tile2world_x(mouse->tile_x),convert_tile2world_y(mouse->tile_y));
					state->towers_length++;
					state->refresh_paths = 1;
				}
				break;
			case DESTROY_TOWER:
				if (state->towers_length != 0) {
					//Overloop de torens om te kijken welke toren er net geselecteerd werd.
					for(i = 0; i < state->towers_length; i++) {
						if (convert_world2tile_x(state->towers[i]->tower.world_x) == mouse->tile_x
							&& convert_world2tile_y(state->towers[i]->tower.world_y) == mouse->tile_y) break;
					}
					//Er staat hier geen toren.
					if(i>=state->towers_length) return;
					//Er staat hier wel een toren, verwijder hem!
					destroy_tower(&state->world,state->towers[i]);
					state->towers[i] = state->towers[state->towers_length-1];
					state->towers = (Entity **) realloc(state->towers,(state->towers_length-1)*sizeof(Entity*));
					state->towers_length--;
					state->refresh_paths = 1;
				}
				break;
			}
		}
		else { /* Outside of world frame => buttons */
			set_button_action(state); /* DO NOT CHANGE */
		}
	}
	else if (ev->button == 2) {
		state->action = NONE;
	}
}

void mouse_up(MouseUpEvent * ev, GameState * state)
{
	reset_button_action(state);
}


/* === Implemented methods, DO NOT CHANGE === */

void set_button_hover_state(GameState * state)
{
	int i;
	Button * buttons = state->hud.buttons;
	Mouse * mouse = &state->mouse;

	for (i = 0; i < BUTTON_AMOUNT; i++) {
		if (in_bounds(mouse->screen_x, mouse->screen_y, buttons[i].bounds)) {
			buttons[i].state = BUTTON_HOVER;
		} 
		else {
			buttons[i].state = BUTTON_UP;
		}
	}
}

void set_button_action(GameState * state)
{
	int i;
	Button * buttons = state->hud.buttons;
	Mouse * mouse = &state->mouse;

	for (i = 0; i < BUTTON_AMOUNT; i++) {
		if (in_bounds(mouse->screen_x, mouse->screen_y, buttons[i].bounds)) {
			buttons[i].state = BUTTON_DOWN;
			switch(i) {
			case BUTTON_TOWER_MACHINEGUN:
				init_tower_blueprint(&state->blueprint.entity, MACHINE_GUN);
				state->action = BUILD_TOWER;
				break;

			case BUTTON_TOWER_ROCKET:
				init_tower_blueprint(&state->blueprint.entity, ROCKET_LAUNCHER);
				state->action = BUILD_TOWER;
				break;

			case BUTTON_TOWER_FLAK:
				init_tower_blueprint(&state->blueprint.entity, FLAK_CANNON);
				state->action = BUILD_TOWER;
				break;
			case BUTTON_DESTROY_TOWER:

				state->action = DESTROY_TOWER;
				break;
			case BUTTON_SPELL_FREEZE:
				state->action = GENERATE_FROST_WAVE;
				break;

			case BUTTON_SPELL_KILL:
				state->action = RELEASE_POISON_GAS;
				break;
			}
		}
	}
}

void reset_button_action(GameState * state)
{
	Mouse * mouse = &state->mouse;
	Button * buttons = state->hud.buttons;
	int i;

	for (i = 0; i < BUTTON_AMOUNT; i++) {
		if (in_bounds(mouse->screen_x, mouse->screen_y, buttons[i].bounds)) {
			buttons[i].state = BUTTON_UP;
		}
	}
}

void reset_all_buttons_action(GameState * state)
{
	Button * buttons = state->hud.buttons;
	int i;

	for (i = 0; i < BUTTON_AMOUNT; i++)
		buttons[i].state = BUTTON_UP;
}