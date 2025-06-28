//
// Created by Kilig on 2024/11/30.
//

#include "memory.h"
#include "closure.h"
#include "bound_method.h"


BoundMethod* new_bound_method(VirtualMachine *vm, Value receiver, Closure *method) {
    BoundMethod *bound = macro_allocate_object(vm, BoundMethod, OBJ_BOUND_METHOD);
    bound->method = method;
    bound->receiver = receiver;
    return bound;
}

void free_bound_method(BoundMethod* self) {
    if (self != NULL) {
        macro_free(self->base.vm, BoundMethod, self);
    }
}

bool bound_method_equal(BoundMethod* left, BoundMethod* right) {
    return left->method == right->method && values_equal(left->receiver, right->receiver);
}

void print_bound_method(BoundMethod* self) {
    if (self != NULL) {
        print_closure(self->method);
    }
}

int snprintf_bound_method(BoundMethod* self, char* buf, size_t size) {
    if (self != NULL) {
        return snprintf_closure(self->method, buf, size);
    }
    return 0;
}


