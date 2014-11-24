#include <LPC17XX.H>
#include <stdio.h>
#include "ball.h"

typedef ball_t list_type;

typedef struct linked_node{
	struct linked_node *next;
	list_type *content;
}linked_node_t;

typedef struct linked_list {
	linked_node_t *root;
	linked_node_t *current;
}linked_list_t;

void list_init(linked_list_t *list);

void list_add(linked_list_t *list, list_type *content);

list_type *list_current(linked_list_t *list);

list_type *list_next(linked_list_t *list);

void list_remove(linked_list_t *list, list_type *content);

list_type *list_reset(linked_list_t *list);