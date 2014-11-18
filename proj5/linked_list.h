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

void list_init( void );

void list_add( void );

list_type *list_current( void );

list_type *list_next( void );

void list_reset( void );