//
// Created by Kilig on 2024/11/21.
//


#include <stdio.h>
#include "common.h"
#include "error.h"
#include "memory.h"
#include "string_.h"
#include "vm.h"
#include "fn.h"
#include "native.h"


Native* new_native(VirtualMachine* vm, NativeFnPtr fn, bool is_builtin_method) {
	Native* native = macro_allocate_object(vm, Native, OBJ_NATIVE);
	native->fn = fn;
    native->is_builtin_method = is_builtin_method;
	return native;
}

void free_native(Native* self) {
	if (self != NULL) {
		macro_free(self->base.vm, Native, self);
	}
}

bool native_equal(Native* left, Native* right) {
    return left->fn == right->fn;
}

void print_native(Native* self) {
	if (self == NULL) {
		panic("[ {PANIC} Native::print_native] Expected parameter 'native' to be non-null, Found null instead.");
	}

	printf("<Native Fn>");
}

int snprintf_native(Native* self, char* buf, size_t size) {
    (void)self;

    return snprintf(buf, size, "<Native Fn>");
}
