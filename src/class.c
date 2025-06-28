//
// Created by Kilig on 2024/11/30.
//
#include <stdio.h>

#include "memory.h"
#include "string_.h"
#include "class.h"


Class* new_class(VirtualMachine *vm, String* name) {
    Class* klass = macro_allocate_object(vm, Class, OBJ_CLASS);
    klass->name = name;
    init_hashmap(&klass->methods, vm);
    return klass;
}

void free_class(Class* self) {
    if (self != NULL) {
        free_hashmap(&self->methods);
        macro_free(self->base.vm, Class, self);
    }
}

bool class_equal(Class* left, Class* right) {
    return string_equal(left->name, right->name);
}

void print_class(Class* self) {
    printf("<class ");
    print_string(self->name);
    printf(">");
}

int snprintf_class(Class* self, char* buf, size_t size) {
    return snprintf(buf, size, "<class %s>", self->name->chars);
}
