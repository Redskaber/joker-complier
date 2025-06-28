//
// Created by Kilig on 2024/11/21.
//
#pragma once

#ifndef JOKER_FN_H
#define JOKER_FN_H
#include "common.h"
#include "object.h"
#include "chunk.h"

#define macro_is_fn(value)		is_obj_type(value, OBJ_FN)
#define macro_is_native(value)	is_obj_type(value, OBJ_NATIVE)
#define macro_is_closure(value) is_obj_type(value, OBJ_CLOSURE)

#define macro_as_fn(value)						((Fn*)macro_as_obj(value))
#define macro_as_fn_from_obj(obj)				((Fn*)obj)

#define macro_as_native(value)                  ((Native*)macro_as_obj(value))
#define macro_as_native_from_obj(obj)			((Native*)obj)
#define macro_as_native_from_vptr(value_ptr)    ((Native*)macro_as_obj_ptr(value_ptr))

#define macro_as_closure(value)                 ((Closure*)macro_as_obj(value))
#define macro_as_closure_from_obj(obj)			((Closure*)obj)
#define macro_as_closure_from_vptr(value_ptr)	((Closure*)macro_as_obj_ptr(value_ptr))


typedef enum FnType {
	type_fn,
    type_method,
    type_initializer,
	type_script,
    type_lambda,
} FnType;

typedef struct Fn {
	Object base;
	int arity;
	Chunk chunk;
	String* name;
	int upvalue_count;
} Fn;

Fn* new_fn(VirtualMachine* vm);
void free_fn(Fn* self);
bool fn_equal(Fn* left, Fn* right);
bool is_anonymous_fn(Fn* self);
void print_fn(Fn* self);
int snprintf_fn(Fn* self, char* buf, size_t size);

#endif //JOKER_FN_H
