// Bart Middag & Thomas Dhondt ; groepsnummer: 14 ; pathfinding.c

#include <stdlib.h>
#include "entities.h"
#include "world.h"
#include "util.h"
#include "pathfinding.h"
#include "config.h"
#include "priority_queue.h"
#include "hash_set.h"
#include <math.h>
#include <stdio.h>

#define DEFAULT_MOVE_COST 10;

/* Useful definitions for neighbour calculation */
#define NR_OF_DIRECTIONS 4
typedef enum {UP, DOWN, LEFT, RIGHT} Directions;
const int x_movement[NR_OF_DIRECTIONS] = {0, 0, -1, 1};
const int y_movement[NR_OF_DIRECTIONS] = {1, -1, 0, 0}; 

/* Node validity function */
int is_valid(World *world, Node *node,  TrajectoryType trajectory_type) {
	Entity entity;
	int valid;
	valid = 0;
	if(node->tile_x >=0 && node->tile_x < world->width_in_tiles && node->tile_y >=0 && node->tile_y < world->height_in_tiles ) {
		entity = world->entities[node->tile_x][node->tile_y];
		if(trajectory_type == OVER_AIR) {
			if ((entity.type != OBSTACLE) || (entity.type == OBSTACLE && entity.obstacle.obstacle_type != MOUNTAIN)) {
				valid = 1;
			}
		} else {
			if (entity.type == EMPTY || entity.type == SPAWN_LOCATION || entity.type == CASTLE) {
				valid = 1;
			}
		}
	}
	return valid;
}

/* Private function */
int find_path(Path * path, World *world, Node *start, Node *goal, TrajectoryType trajectory_type)
{
	int i, j, g, h, f;
	PriorityQueue open;
	HashSet closed; 
	Node neighbour, *current, *open_node;
	Element *open_neighbour;

	pq_init(&open);
	hs_init(&closed);

	// voeg huidige node aan open toe
	pq_offer(&open,start);

	current = pq_poll(&open);
	while(current != NULL) {
		// current aan closed toevoegen
		hs_add(&closed,current);

		// enum overlopen
		for (i = UP; i<=RIGHT; i++) {
			neighbour.tile_x = current->tile_x + x_movement[i];
			neighbour.tile_y = current->tile_y + y_movement[i];

			if(is_valid(world,&neighbour,trajectory_type) && !hs_contains(&closed,&neighbour)) {
				g = current->score.g + DEFAULT_MOVE_COST;
				h = (labs(neighbour.tile_x - goal->tile_x) + labs(neighbour.tile_y - goal->tile_y))*DEFAULT_MOVE_COST;
				f = g + h;

				if(pq_contains(&open,&neighbour)) {
					//Zoek de node in de priority queue open.
					open_neighbour = open.head;
					if(open_neighbour->next != open.tail) open_neighbour = open_neighbour->next;
					while (open_neighbour->next != open.tail && (hs_calc_hash(open_neighbour->value)!=hs_calc_hash(&neighbour))) {
						open_neighbour = open_neighbour->next;
					}
					open_node = open_neighbour->value;

					//Pas indien nodig de node in de priority queue open aan.
					if(g<open_node->score.g) {
						open_node->parent = current;
						open_node->score.g = g;
						open_node->score.h = h;
						open_node->score.f = f;
						pq_remove(&open,open_node);
						pq_offer(&open,open_node);
					}
				} else {
					//Voeg de node aan open toe.
					Node* add_neighbour = (Node*)malloc(sizeof(Node));
					add_neighbour->parent = current;
					add_neighbour->score.g = g;
					add_neighbour->score.h = h;
					add_neighbour->score.f = f;
					add_neighbour->tile_x = neighbour.tile_x;
					add_neighbour->tile_y = neighbour.tile_y;
					pq_offer(&open,add_neighbour);
				}
			}
		}
		if(!pq_contains(&open,goal)) {
			current = pq_poll(&open);
		} else current = NULL;
	}
	if (pq_contains(&open,goal)) {
		//Zoek de goal-node in de priority queue open.
		open_neighbour = open.head;
		if(open_neighbour->next != open.tail) {
			open_neighbour = open_neighbour->next;
		}
		//Neighbour, de laatste node, is de goal-node, dus we kunnen daarmee vergelijken.
		while (open_neighbour->next != open.tail && (hs_calc_hash(open_neighbour->value)!=hs_calc_hash(&neighbour))) {
			open_neighbour = open_neighbour->next;
		}
		//Tel het aantal nodes op het pad van goal naar start. Dit wordt i.
		current = open_neighbour->value;
		for(i = 0; current!=NULL; i++) {
			current = current->parent;
		}
		if(i>0) {
			path->nodes = (Node*) malloc(i*sizeof(Node));
		}
		current = open_neighbour->value;

		for(j = i-1; j>=0; j--) {
			//Hardcopy van het pad dat gevonden is
			if(j<i-1) path->nodes[j+1].parent = &path->nodes[j];
			path->nodes[j].parent = NULL;
			path->nodes[j].score.f = current->score.f;
			path->nodes[j].score.h = current->score.h;
			path->nodes[j].score.g = current->score.g;
			path->nodes[j].tile_x = current->tile_x;
			path->nodes[j].tile_y = current->tile_y;
			current = current->parent;
		}
		//Verwijder de datastructuren. Doordat alle aangemaakte nodes in één van de twee zitten,
		//worden deze nodes ook verwijderd.
		pq_destroy(&open);
		hs_destroy(&closed);
		return i;
	}
	pq_destroy(&open);
	hs_destroy(&closed);
	return 0;
}


int create_path(Path * path, World * world, Entity * from, Entity * to, TrajectoryType trajectory_type) 
{
	Node* start;
	Node goal;
	int i;

	//Start & goal nodes maken adhv "from" en "to" entities
	start = (Node*) malloc(sizeof(Node));
	start->tile_x = convert_world2tile_x(from->all.world_x);
	start->tile_y = convert_world2tile_y(from->all.world_y);
	start->score.g = 0;
	start->score.f = 0;
	start->parent = NULL;

	// We moeten goal niet onthouden na deze methode, dus geen alloc!
	goal.tile_x = convert_world2tile_x(to->all.world_x);
	goal.tile_y = convert_world2tile_y(to->all.world_y);

	//Voor we beginnen controleren we of de from en to wel bereikbaar zijn...
	if (!is_valid(world,start,trajectory_type) || !is_valid(world,&goal,trajectory_type)) {
		free(start);
		return 0;
	}

	//Als start en goal dezelfde zijn, is het tijd om 1 te returnen!
	if(hs_calc_hash(start) == hs_calc_hash(&goal)) {
		path->length = 1;
		path->current_node_index=0;
		path->nodes = start;
		path->trajectory_type = trajectory_type;
		return 1;
	}

	//Maak het pad aan
	i = find_path(path,world,start,&goal,trajectory_type);
	path->trajectory_type = trajectory_type;
	path->current_node_index = 0;
	path->length = i;
	return (i>0);
}

int refresh_path(Path * path, World * world)
{
	//Maak een nieuw pad van de current_node naar de castle entity.
	int x, y, end_x, end_y;
	x = path->nodes[path->current_node_index].tile_x;
	y = path->nodes[path->current_node_index].tile_y;
	end_x = convert_world2tile_x(world->castle->all.world_x);
	end_y = convert_world2tile_y(world->castle->all.world_y);
	destroy_path(path);
	return create_path(path,world,&world->entities[x][y],&world->entities[end_x][end_y],path->trajectory_type);
}

void destroy_path(Path * path)
{
	if(path->nodes != NULL) free(path->nodes);
	path->nodes = NULL;
	path->current_node_index = 0;
	path->length = 0;
}
