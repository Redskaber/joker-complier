//
// Created by Kilig on 2024/11/21.
//

#include <string.h>

#include "memory.h"
#include "option.h"
#include "string_.h"
#include "hashmap.h"
#include "vec.h"


static Entry* find_entry(Entry* entries, int capacity, String* key);
static void adjust_capacity(HashMap* self, int new_capacity);

bool is_empty_entry(Entry* entry) {
	return entry->key == NULL && macro_is_null(entry->value);
}

void init_hashmap(HashMap* self, VirtualMachine *vm) {
    self->vm = vm;
	self->count = 0;
	self->capacity = 0;
	self->entries = NULL;
}

void free_hashmap(HashMap* self) {
	macro_free_array(self->vm, Entry, self->entries, self->capacity);
    self->vm = NULL;
    self->count = 0;
    self->capacity = 0;
    self->entries = NULL;
}

/* return bool? return find entry? */
bool hashmap_set(HashMap* self, String* key, Value value) {
	if (self->count + 1 > self->capacity * const_max_load_factor) {
		int new_capacity = macro_grow_capacity(self->capacity);
		adjust_capacity(self, new_capacity);
	}
	Entry* entry = find_entry(self->entries, self->capacity, key);
	bool is_new_entry = entry->key == NULL;
	if (is_new_entry && macro_is_null(entry->value)) self->count++; // new entry, count++
	entry->key = key;
	entry->value = value;
	return is_new_entry;
}

/*
* find_entry: find entry in entries by key.
*   - return Entry*: entry if found, else NULL.
*   - used string self hash % capacity to index entries.
*      - if entry->key == key, return entry.
*      - if entry->key != key, used open addressing to find next entry. [(index +1) % capacity].
*      - if find entry open addressing find next entry->key == NULL, open addressing over, return NULL.
*
* used tombstone to mark a deleted entry.
*   - note(log) it and keep going.
*
* how tombstones interact with the table��s load factor and resizing?
*   - tombstone: we used tombstone to one entry, not decrease load factor.(avoid deap loop)
*/
static Entry* find_entry(Entry* entries, int capacity, String* key) {
    // TODO: OPTIMIZE: 22s -> 16s (up prent 27%)
    // TODO: OPTIMIZE: use hash_code(key) % capacity => hash_code(key) & (capacity - 1)
    // uint32_t index = key->hash % capacity;
    uint32_t index = key->hash & (capacity - 1);
	Entry* tombstone = NULL;

	while (true) {
		Entry* entry = &entries[index];
		// handler tombstone
		if (entry->key == NULL) {
			if (macro_is_null(entry->value)) {
				return tombstone != NULL ? tombstone : entry;
			}
			else {
				// tombstone: {key: null, value: true(bool)}
				if (tombstone == NULL) tombstone = entry;
			}
		}
		else if (entry->key == key) {
			return entry;
		}
		// index = (index + 1) % capacity;
        index = (index + 1) & (capacity - 1);
	} // dead loop?: no! because exist load factor, so have entry->key == NULL.
}

// allocate new entries and copy old entries to new entries
static void adjust_capacity(HashMap* self, int new_capacity) {
	Entry* new_entries = macro_allocate(self->vm, Entry, new_capacity);
	// init new_entries
	for (int i = 0; i < new_capacity; i++) {
		new_entries[i].key = NULL;
		new_entries[i].value = macro_val_null;
	}
	// copy old entries to new entries + reset count(!tombstone)
	self->count = 0;
	for (int i = 0; i < self->capacity; i++) {
		Entry* entry = &self->entries[i];
		if (entry->key != NULL) {
			Entry* new_entry = find_entry(new_entries, new_capacity, entry->key);
			new_entry->key = entry->key;
			new_entry->value = entry->value;
			self->count++;
		}
	}
	// free old entries
	macro_free_array(self->vm, Entry, self->entries, self->capacity);
	// assign new entries to map
	self->entries = new_entries;
	self->capacity = new_capacity;
}

// add all entries from from_map to to_map
void hashmap_add_all(HashMap* from, HashMap* to) {
	for (int i = 0; i < from->capacity; i++) {
		Entry* entry = &from->entries[i];
		if (entry->key != NULL) {
			hashmap_set(to, entry->key, entry->value);
		}
	}
}
/* note: this function can return one block of memory, so use over it need to free it. */
Value hashmap_get(HashMap* self, String* key) {
	if (self->count == 0) return macro_val_null;
	Entry* entry = find_entry(self->entries, self->capacity, key);
	return entry->key == NULL ? macro_val_null : entry->value;
}

/*
* note: used open addressing, so if set value to null is bad idea.used tombstone instead.
*    - tombstone: a special value to mark a deleted entry.
*    - when remove an entry, set its key to null and value to true(bool).
*/
bool hashmap_remove(HashMap* self, String* key) {
	if (self->count == 0) return false;
	Entry* entry = find_entry(self->entries, self->capacity, key);
	if (entry->key == NULL) return false;

	entry->key = NULL;
	entry->value = macro_val_from_bool(true);
	// map->count--;  // because, we use tombstone instead. tombstone count not need to decrease.
	return true;
}

/* note: this function used to find a key in hashmap, but not used to set value. */
String* hashmap_find_key(HashMap* self, const char* key, uint32_t len, uint32_t hash) {
	if (self->count == 0) return NULL;

    // TODO: OPTIMIZE: use hash_code(key) % capacity => hash_code(key) & (capacity - 1)
    // uint32_t index = hash % self->capacity; optimize: 22s -> 16s (up prent 27%)
    uint32_t index = hash & (self->capacity -1);
	while (true) {
		Entry* entry = &self->entries[index];
		if (entry->key == NULL) {
			if (macro_is_null(entry->value)) return NULL; // non-tombstone, return null.
		}
		else if (
			entry->key->length == (int)len &&
			entry->key->hash == hash &&
			memcmp(entry->key->chars, key, len) == 0
			) {
			return entry->key; // found
		}
		// index = (index + 1) % self->capacity;
        index = (index + 1) & (self->capacity - 1);
	}
}

bool hashmap_contains_key(HashMap* self, String* key) {
	if (self->count == 0) return false;
	Entry* entry = find_entry(self->entries, self->capacity, key);
	return entry->key != NULL;
}


Vec* hashmap_keys(HashMap* self) {
    Vec* keys = new_vec_with_capacity(self->vm, self->count);
    for(int i = 0; i < self->capacity; i++) {
        if(self->entries[i].key != NULL) {
            vec_push(keys, macro_val_from_obj(self->entries[i].key));
        }
        if (vec_len(keys) == (size_t)self->count) break;
    }
    return keys;
}

Vec* hashmap_values(HashMap* self) {
    Vec *values = new_vec_with_capacity(self->vm, self->count);
    for(int i = 0; i < self->capacity; i++) {
        if(self->entries[i].key != NULL) {
            vec_push(values, self->entries[i].value);
        }
        if (vec_len(values) == (size_t)self->count) break;
    }
    return values;
}

bool hashmap_equal(HashMap* left, HashMap* right) {
    if (left->count != right->count) return false;
    for (int i = 0; i < left->capacity; i++) {
        if (left->entries[i].key != NULL) {
            if (!hashmap_contains_key(right, left->entries[i].key)) return false;
        }
    }
    return true;
}

/* note: this function used to get entry in hashmap, if not found return new entry. */
Entry* hashmap_get_entry(HashMap* self, String* key) {
	if (self->count + 1 > self->capacity * const_max_load_factor) {
		int new_capacity = macro_grow_capacity(self->capacity);
		adjust_capacity(self, new_capacity);
	}
	return find_entry(self->entries, self->capacity, key);
}
