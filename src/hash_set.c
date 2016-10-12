// Bart Middag & Thomas Dhondt ; groepsnummer: 14 ; hash_set.c

#include <stdlib.h>
#include "hash_set.h"
#include <stdio.h>

void hs_init(HashSet * hs)
{
	int i;
	hs->size = 0;
	hs->nr_of_buckets = DEFAULT_BUCKETS;
	hs->buckets = (HashEntry**) calloc(hs->nr_of_buckets, sizeof(HashEntry*));
	for(i = 0; i<hs->nr_of_buckets; i++) {
		//We hebben telkens één element dat een verwijzing bijdhoudt naar het echte eerste element.
		//Dit is een stuk handiger om mee te werken.
		//Hier doen we calloc, zodat alles op 0 staat.
		hs->buckets[i] = (HashEntry*) calloc(1,sizeof(HashEntry));
	}
}

void hs_add(HashSet * hs, Node * node)
{
	HashEntry* entry;
	HashEntry* new_entry;

	if(hs_contains(hs,node)) return;

	entry = hs->buckets[hs_calc_hash(node) % hs->nr_of_buckets];

	//We maken het nieuwe element en we voegen het vooraan toe
	new_entry = (HashEntry*) malloc(sizeof(HashEntry));
	new_entry->next = entry->next;
	new_entry->value = node;
	entry->next = new_entry;

	//De HashSet wordt groter, moeten we al rehashen?
	hs->size++;
	hs_rehash(hs);
}

int hs_contains(HashSet *hs, Node * node)
{
	HashEntry* entry;
	int hash;
	hash = hs_calc_hash(node);
	entry = hs->buckets[hash % hs->nr_of_buckets];
	while(entry->next != NULL) {
		if(hs_calc_hash(entry->next->value) == hash) {
			return 1;
		} else {
			entry = entry->next;
		}
	}
	return 0;
}

void hs_remove(HashSet * hs, Node * node)
{
	HashEntry* entry;
	HashEntry* remove;
	int hash;
	hash = hs_calc_hash(node);
	entry = hs->buckets[hs_calc_hash(node) % hs->nr_of_buckets];

	// Overloop de elementen. Zit het ertussen -> verwijder het.
	while(entry->next != NULL) {
		if(hs_calc_hash(entry->next->value) == hash) {
			remove = entry->next;
			entry->next = entry->next->next;
			free(remove);
			hs->size--;
			hs_rehash(hs);
			return;
		} else {
			entry = entry->next;
		}
	}
}

Node * hs_get_node(HashSet * hs, int tile_x, int tile_y)
{
	HashEntry* entry;
	int hash;
	hash = (tile_x << 16) | (tile_y & 0xFFFF);
	entry = hs->buckets[hash % hs->nr_of_buckets];
	while(entry->next != NULL) {
		if(hs_calc_hash(entry->next->value) == hash) {
			return entry->next->value;
		} else {
			entry = entry->next;
		}
	}
	return NULL;
}

/* Geeft 0 terug als er niet opnieuw gehasht moet worden
   Geeft de nieuwe aantal buckets terug als er wel opnieuw gehasht moet worden 
   Already implemented, do not change!
*/
int rehash_nr_of_buckets(const HashSet* hs) {
	if (hs->size > 8 * hs->nr_of_buckets) {
		return 16 * hs->nr_of_buckets;
	}
	else if (hs->nr_of_buckets > DEFAULT_BUCKETS && hs->nr_of_buckets > 8 * hs->size) {
		return hs->nr_of_buckets / 16;
	}
	return 0;
}

// Functie kijkt eerst of er gehasht moet worden, en als dit moet doet hij het ook.
void hs_rehash(HashSet * hs)
{
	int nr, i;
	HashEntry** buckets;
	HashEntry* entry;
	HashEntry* next;
	HashEntry* bucket;

	nr = rehash_nr_of_buckets(hs);
	if(!nr) return;

	buckets = (HashEntry**) calloc(nr, sizeof(HashEntry*));
	for(i = 0; i < nr; i++) {
		buckets[i] = (HashEntry*) calloc(1,sizeof(HashEntry));
	}

	for(i = 0; i < hs->nr_of_buckets; i++) {
		entry = hs->buckets[i];
		while(entry->next != NULL) {
			next = entry->next->next;
			bucket = buckets[hs_calc_hash(entry->next->value) % nr];
			entry->next->next = bucket->next;
			bucket->next = entry->next;
			entry->next = next;
		}
		free(hs->buckets[i]);
	}
	free(hs->buckets);
	hs->buckets = buckets;
	hs->nr_of_buckets = nr;
}

unsigned int hs_calc_hash(Node * node)
{
	return (node->tile_x << 16) | (node->tile_y & 0xFFFF);
}

void hs_destroy(HashSet * hs)
{
	int i;
	HashEntry* entry;
	HashEntry* next;
	for(i = 0; i<hs->nr_of_buckets; i++) {
		entry = hs->buckets[i];
		while(entry->next!=NULL) {
			next = entry->next->next;
			//Free ook de nodes - anders moeten we die bij pathfinding bijhouden en het wordt toch enkel daar gebruikt!
			free(entry->next->value);
			free(entry->next);
			entry->next = next;
		}
		free(entry);
	}
	free(hs->buckets);
}