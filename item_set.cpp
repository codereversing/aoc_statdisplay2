#include "item_set.h"
#include <stdio.h>
#include <Windows.h>
#include "player_stats.h"

bool items_insert(item_set** head, item_set* node) {
	if(node == NULL)
		return false;
	if(*head == NULL) {
		*head = node;
		(*head)->next = NULL;
		return true;
	}
	item_set* node_ptr = *head;
	item_set* prev_ptr = *head;
	while(node_ptr != NULL) {
		if(node_ptr->base_pointer == node->base_pointer)
			return false;
		prev_ptr = node_ptr;
		node_ptr = node_ptr->next;
	}
	prev_ptr->next = node;
	return true;
}

item_set* items_find_by_name(item_set** head, char* name) {
    if(*head == NULL)
        return NULL;
    item_set *node_ptr = *head;
    while(node_ptr != NULL) {
        if(strstr(name, node_ptr->player_name) != NULL)
            return node_ptr;
        node_ptr = node_ptr->next;
    }
    return NULL;
}

void items_print(item_set** head) {
	if(*head == NULL)
		return;
	item_set *node_ptr = *head;
    while(node_ptr != NULL) {
        printf("Player: %s -- Wood: %1.0f - Food %1.0f - Gold: %1.0f - Stone: %1.0f\n", node_ptr->player_name,
            node_ptr->player_stat->wood, node_ptr->player_stat->food, node_ptr->player_stat->gold, node_ptr->player_stat->stone);
		node_ptr = node_ptr->next;
	}
	printf("\n");
}

void items_clear(item_set** head) {
    if(*head == NULL)
        return;
    item_set* node_ptr = *head;
    item_set* next_ptr = NULL;
    while(node_ptr != NULL) {
        next_ptr = node_ptr->next;
        HeapFree(GetProcessHeap(), 0, node_ptr);
        node_ptr = next_ptr;
    }
    *head = NULL;
}