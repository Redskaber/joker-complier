//
// Created by Kilig on 2024/11/21.
//

#include <stdio.h>

#include "common.h"
#include "memory.h"
#include "Fn.h"
#include "string_.h"
#include "closure.h"

#define macro_allocate_closure(vm, up_count) \
    (Closure*)allocate_object(               \
        vm,                                  \
        (sizeof(Closure) + sizeof(Upvalue*) * up_count), \
        OBJ_CLOSURE                          \
    )

/* upvalue_ptrs: { upvalue_ptr, upvalue_ptr, upvalue_ptr, } */
Closure* new_closure(Fn* fn) {

	Closure* closure = macro_allocate_closure(fn->base.vm, fn->upvalue_count);
	closure->fn = fn;
	closure->upvalue_ptrs = NULL;
	closure->upvalue_count = fn->upvalue_count;

    /* init upvalues for after Closure */
    if (fn->upvalue_count > 0) {
        closure->upvalue_ptrs = (UpvaluePtrs)((char*)closure + sizeof(Closure));
        for (int i = 0; i < fn->upvalue_count; i++) {
            closure->upvalue_ptrs[i] = NULL;
        }
    }
	return closure;
}

void free_closure(Closure* self) {
	if (self != NULL) {
        macro_free(self->base.vm, Closure, self);
    }
}

bool closure_equal(Closure* left, Closure* right) {
    return fn_equal(left->fn, right->fn);
}

void print_closure(Closure* self) {
	if (self == NULL) {
		printf("<closure[NULL]>");
		return;
	}
	print_fn(self->fn);
}

int snprintf_closure(Closure* self, char* buf, size_t size) {
    if (self == NULL) {
        return snprintf(buf, size, "<closure[NULL]>");
    }
    return snprintf_fn(self->fn, buf, size);
}
