#include <LPC17XX.H>
#include <stdio.h>
#include "linked_list.h"

void list_init(linked_list_t *list) {
	list->root = NULL;
	list->current = list->root;
}

void list_add(linked_list_t *list, list_type *content) {
	linked_node_t *newNode;
	do {
		newNode = (linked_node_t *)malloc(sizeof(linked_node_t));
	} while(!newNode);
	
	newNode->content = content;
	
	newNode->next = list->root;
	list->root = newNode;
	list->current = newNode;
}

list_type *list_current(linked_list_t *list) {
	if(list->current != NULL) {
		return list->current->content;
	}
	return NULL;
}

list_type *list_next(linked_list_t *list) {
	if(list->current != NULL && list->current->next != NULL) {
		list->current = list->current->next;
		return list->current->content;
	}
	return 0;
}

list_type *list_reset(linked_list_t *list) {
	list->current = list->root;
	if(list->current) {
		return list->current->content;
	}
	return NULL;
}