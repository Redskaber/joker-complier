//
// Created by Kilig on 2024/11/21.
//
#pragma once

#ifndef JOKER_CLOSURE_H
#define JOKER_CLOSURE_H
#include "common.h"
#include "object.h"

typedef struct Closure {
	Object base;				// object base
	Fn* fn;						// function object
	int upvalue_count;			// upvalue count
	Upvalue**  upvalue_ptrs;	// upvalue pointer array (used heap memory, once Upvalue pointer) [upvalue link list]
} Closure;

Closure* new_closure(Fn* fn);
void free_closure(Closure* self);
bool closure_equal(Closure* left, Closure* right);
void print_closure(Closure* self);
int snprintf_closure(Closure* self, char* buf, size_t size);

#endif //JOKER_CLOSURE_H
