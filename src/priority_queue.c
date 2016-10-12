// Bart Middag & Thomas Dhondt ; groepsnummer: 14 ; priority_queue.c

#include <stdlib.h>
#include "entities.h"
#include "priority_queue.h"
#include "hash_set.h"

void pq_remove_element(PriorityQueue * pq, Element * element);

void pq_init(PriorityQueue * pq)
{
	Element* head;
	Element* tail;
	//Head en tail zijn twee elementen die eigenlijk gewoon verwijzingen zijn naar de echte elementen in de PQ.
	//Er zijn dus automatisch 2 'onechte' elementen aanwezig. Dit is veel handiger om mee te werken!
	//We doen hier expliciet calloc, geen malloc, omdat we geen problemen willen met slechte waarden.
	head = (Element*) calloc(1,sizeof(Element));
	tail = (Element*) calloc(1,sizeof(Element));
	head->next = tail;
	tail->previous = head;
	pq->head = head;
	pq->tail = tail;
	pq->size = 0;
}

void insert_before(PriorityQueue * pq, Element * new_element, Element * target_element)
{
	target_element->previous->next = new_element;
	new_element->previous = target_element->previous;
	new_element->next = target_element;
	target_element->previous = new_element;
	pq->size++;
}

void insert_after(PriorityQueue * pq, Element * new_element, Element * target_element)
{
	target_element->next->previous = new_element;
	new_element->next = target_element->next;
	new_element->previous = target_element;
	target_element->next = new_element;
	pq->size++;
}

void pq_offer(PriorityQueue * pq, Node * node)
{
	Element* new_element;
	Element* current;

	new_element = (Element*) malloc(sizeof(Element));
	new_element->value = node;
	new_element->priority = node->score.f;

	current = pq->head;
	while(current->next != pq->tail) {
		if(current->next->priority > new_element->priority) break;
		current = current->next;
	}
	insert_before(pq,new_element,current->next);
}

int pq_is_empty(PriorityQueue * pq)
{
	return (pq->size == 0);
}

Node * pq_poll(PriorityQueue * pq)
{
	Node* node;
	if(pq->head->next == pq->tail) return NULL;
	node = pq->head->next->value;
	pq_remove_element(pq,pq->head->next);
	return node;
}

Node * pq_peek(PriorityQueue * pq)
{
	Node* node;
	if(pq->head->next == pq->tail) return NULL;
	node = pq->head->next->value;
	return node;
}

int pq_contains(PriorityQueue * pq, Node * node)
{
	Element* current;
	current = pq->head;
	while(current->next != pq->tail) {
		if(hs_calc_hash(current->next->value) == hs_calc_hash(node)) return 1;
		current = current->next;
	}
	return 0;
}

void pq_remove_element(PriorityQueue * pq, Element * element)
{
	element->next->previous = element->previous;
	element->previous->next = element->next;
	free(element);
	pq->size--;
}

void pq_remove(PriorityQueue * pq, Node * node)
{
	Element* current;
	current = pq->head;
	while(current->next != pq->tail) {
		if(hs_calc_hash(current->next->value) == hs_calc_hash(node)) {
			pq_remove_element(pq,current->next);
			return;
		}
		current = current->next;
	}
}

void pq_destroy(PriorityQueue * pq)
{
	Element* current;
	current = pq->head;
	while(current->next != pq->tail) {
		//Free ook de nodes - anders moeten we die bij pathfinding bijhouden en het wordt toch enkel daar gebruikt!
		free(current->next->value);
		pq_remove_element(pq,current->next);
	}
	free(pq->head);
	free(pq->tail);
}