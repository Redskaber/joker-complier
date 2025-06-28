//
// Created by Kilig on 2024/12/14.
//

#include <stdbool.h>

#include "memory.h"
#include "vec.h"
#include "enum.h"
#include "enum_instance.h"

#define macro_allocate_enum_instance(vm, capacity) \
    (EnumInstance*)allocate_object(vm, (sizeof(EnumInstance) + sizeof(Vec) + capacity * sizeof(Value)), OBJ_ENUM_INSTANCE)


EnumInstance* new_enum_instance(VirtualMachine *vm, Enum* enum_type, String* name, int32_t index, int capacity) {
    EnumInstance* instance = macro_allocate_enum_instance(vm, capacity);
    instance->enum_type = enum_type;
    instance->index = index;    // store index for in enum type
    instance->name = name;      // store name for in enum type
    instance->values = NULL;    // store values
    if (capacity > 0) {
        instance->values = (Vec*)((char*)instance + sizeof(EnumInstance));
        instance->values->base.vm = vm;
        instance->values->base.type = OBJ_VEC;
        instance->values->base.is_marked = false;
        instance->values->base.next = NULL;
        instance->values->start = (Value*)((char*)instance->values + sizeof(Vec));
        instance->values->finish = instance->values->start + capacity;
        instance->values->end = instance->values->start + capacity;
    }
    return instance;
}

void print_enum_instance(EnumInstance *self) {
    if (self != NULL) {
        print_enum(self->enum_type);
    }
}

void free_enum_instance(EnumInstance *self) {
    if (self != NULL) {
        macro_free(self->base.vm, EnumInstance, self);
    }
}

inline bool enum_instance_equal(EnumInstance *left, EnumInstance *right) {
    return left->enum_type == right->enum_type && left->index == right->index;
}

inline bool enum_values_is_empty(EnumInstance* self) {
    return self->values == NULL;
}

int snprintf_enum_instance(EnumInstance *self, char *buf, size_t size) {
    return snprintf_enum(self->enum_type, buf, size);
}


