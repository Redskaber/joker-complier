//
// Created by Kilig on 2024/11/21.
//
#pragma once

#ifndef JOKER_HASHMAP_H
#define JOKER_HASHMAP_H
#include "common.h"
#include "value.h"
#include "option.h"

#define const_max_load_factor 0.75


/* TODO: trait impl generic for HashMap */
typedef struct Entry {
	String* key;
	Value value;
} Entry;

bool is_empty_entry(Entry* entry);

typedef struct HashMap {
    VirtualMachine *vm;
	int count;
	int capacity;
	Entry* entries;
} HashMap;

void init_hashmap(HashMap* self, VirtualMachine *vm);
void free_hashmap(HashMap* self);
bool hashmap_set(HashMap* self, String* key, Value value);
void hashmap_add_all(HashMap* from, HashMap* to);
Value hashmap_get(HashMap* self, String* key);
bool hashmap_remove(HashMap* self, String* key);
String* hashmap_find_key(HashMap* self, const char* key, uint32_t len, uint32_t hash);
bool hashmap_contains_key(HashMap* self, String* key);
Vec* hashmap_keys(HashMap* self);
Vec* hashmap_values(HashMap* self);
bool hashmap_equal(HashMap* left, HashMap* right);

Entry* hashmap_get_entry(HashMap* self, String* key);

#endif //JOKER_HASHMAP_H
