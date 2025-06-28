//
// Created by Kilig on 2024/11/30.
//

#include <string.h>

#include "memory.h"
#include "hashmap.h"
#include "object.h"
#include "string_.h"
#include "class.h"
#include "vec.h"
#include "instance.h"



Instance *new_instance(VirtualMachine *vm, Class* klass) {
    Instance *instance = macro_allocate_object(vm, Instance, OBJ_INSTANCE);
    instance->klass = klass;
    instance->base.vtable = klass->base.vtable;

    init_hashmap(&instance->fields, vm);
    return instance;
}

void free_instance(Instance* self) {
    if (self != NULL) {
        free_hashmap(&self->fields);
        macro_free(self->base.vm, Instance, self);
    }
}

bool instance_equal(Instance* left, Instance* right) {
    return string_equal(left->klass->name, right->klass->name);
}

void print_instance(Instance* self) {
    printf("<%s instance>", self->klass->name->chars);
}

int snprintf_instance(Instance* self, char* buf, size_t size) {
    if(strcmp(self->klass->name->chars, "Vec") == 0) {
        Value data_val = hashmap_get(&self->fields, new_string(self->base.vm, "_data", 5));
        return snprintf_vec(macro_as_vec(data_val), buf, size);
    }
    return snprintf(buf, size, "<%s instance>", self->klass->name->chars);
}

