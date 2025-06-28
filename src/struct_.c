//
// Created by Kilig on 2024/12/9.
//
#include <stdio.h>
#include <stdbool.h>

#include "memory.h"
#include "vec.h"
#include "string_.h"
#include "struct_.h"


Struct* new_struct(VirtualMachine *vm, String* name) {
    Struct* self = macro_allocate_object(vm, Struct, OBJ_STRUCT);
    self->name = name;
    self->count = 0;
    init_hashmap(&self->fields, vm);
    self->names = new_vec(vm);
    return self;
}

void free_struct(Struct* self) {
    if (self != NULL) {
        free_hashmap(&self->fields);
        // free_vec(self->names);   // TODO: fix this, free struct auto free
        macro_free(self->base.vm, Struct, self);
    }
}

bool struct_equal(Struct* left, Struct* right) {
    return string_equal(left->name, right->name);
}

void print_struct(Struct* self) {
    printf("<struct ");
    print_string(self->name);
    printf(">");
}

int snprintf_struct(Struct* self, char* buf, size_t size) {
    return snprintf(buf, size, "<struct %s>", as_cstring(self->name));
}
