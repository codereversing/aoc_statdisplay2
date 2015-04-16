#pragma once

#include <Windows.h>
#include "player_stats.h"

typedef struct ITEM_SET {
	int* base_pointer;
    char* player_name;
	player_stats* player_stat;
	ITEM_SET* next;
} item_set, *pitem_set;

bool items_insert(item_set** head, item_set* node);
item_set* items_find_by_name(item_set** head, char *name);
void items_print(item_set** head);
void items_clear(item_set** head);