//
// Created by Kilig on 2024/12/12.
//

#include <stdio.h>

#include "memory.h"
#include "string_.h"
#include "vec.h"
#include "enum.h"

Enum *new_enum(VirtualMachine *vm, String *name) {
    Enum* enum_ = macro_allocate_object(vm, Enum, OBJ_ENUM);
    enum_->name = name;
    init_hashmap(&enum_->members, vm);
    return enum_;
}

void print_enum(Enum *self) {
    if (self != NULL) {
        printf("<enum %s>", as_cstring(self->name));
    }
}

void free_enum(Enum *self) {
    if (self != NULL) {
        macro_free(self->base.vm, Enum, self);
    }
}

bool enum_equal(Enum *left, Enum *right) {
    return left == right;
}

int snprintf_enum(Enum *self, char *buf, size_t size) {
    return snprintf(buf, size, "<enum %s>", as_cstring(self->name));
}


