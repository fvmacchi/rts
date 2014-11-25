#include <LPC17XX.H>
#include <stdio.h>
#include "linked_list.h"

void list_init(linked_list_t *list) {
	list->root = NULL;
	list->current = list->root;
}

// Adds new node to beginning of list
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

// Returns the currently selected content
list_type *list_current(linked_list_t *list) {
	if(list->current != NULL) {
		return list->current->content;
	}
	return NULL;
}

// Returns the next in the list
list_type *list_next(linked_list_t *list) {
	if(list->current != NULL && list->current->next != NULL) {
		list->current = list->current->next;
		return list->current->content;
	}
	return 0;
}

// Removes content from the list
void list_remove(linked_list_t *list, list_type *content) {
	linked_node_t *current = list->root;
	linked_node_t *previous = NULL;
	while(current) {
		if(current->content == content) {
			// Remove
			if(!previous) {
				list->root = current->next;
			}
			else {
				previous->next = current->next;
			}
			free(current);
			list_reset(list);
			return;
		}
		previous = current;
		current = current->next;
	}
}

// Sets the current to the root of the list
list_type *list_reset(linked_list_t *list) {
	list->current = list->root;
	if(list->current) {
		return list->current->content;
	}
	return NULL;
}