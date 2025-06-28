//
// Created by Kilig on 2024/11/21.
//


#include <stdio.h>

#include "memory.h"
#include "chunk.h"
#include "string_.h"

#include "error.h"
#include "fn.h"

Fn* new_fn(VirtualMachine* vm) {
	Fn* fn = macro_allocate_object(vm, Fn, OBJ_FN);
	fn->arity = 0;
	fn->name = NULL;
	fn->upvalue_count = 0;
	init_chunk(&fn->chunk, vm);
	return fn;
}

void free_fn(Fn* self) {
	if (self != NULL) {
		free_chunk(&self->chunk);
		macro_free(self->base.vm, Fn, self);
	}
}

bool fn_equal(Fn* left, Fn* right) {
    return string_equal(left->name, right->name);
}

bool is_anonymous_fn(Fn* self) {
	return self->name == NULL;
}

void print_fn(Fn* self) {
	if (self == NULL) {
		panic("[ {PANIC} Function::print_function] Expected parameter 'function' to be non-null, Found null instead.");
		return;
	}

	if (is_anonymous_fn(self)) {
		printf("<script>");
		return;
	}

	printf("<Fn %s(args: %d)>", self->name->chars, self->arity);
}

int snprintf_fn(Fn* self, char* buf, size_t size) {
    if (self == NULL) {
        panic("[ {PANIC} Function::print_function] Expected parameter 'function' to be non-null, Found null instead.");
        return 0;
    }

    if (is_anonymous_fn(self)) {
        return snprintf(buf, size, "<script>");
    }

    return snprintf(buf, size, "<Fn %s(args: %d)>", self->name->chars, self->arity);
}
