//
// Created by Kilig on 2024/11/30.
//
#pragma once

#ifndef JOKER_INSTANCE_H
#define JOKER_INSTANCE_H
#include "common.h"
#include "object.h"
#include "hashmap.h"

#define macro_is_instance(value)   is_obj_type(value, OBJ_INSTANCE)
#define macro_as_instance(value)   ((Instance*)macro_as_obj(value))
#define macro_as_instance_from_obj(object)  ((Instance*)object)
#define macro_as_instance_from_value_ptr(value_ptr)  macro_as_instance_from_obj(macro_as_obj_ptr(value_ptr))

typedef struct Instance {
    Object base;
    Class* klass;
    HashMap fields;
} Instance;

Instance *new_instance(VirtualMachine *vm, Class* klass);
bool instance_equal(Instance* left, Instance* right);
void free_instance(Instance* self);
void print_instance(Instance* self);
int snprintf_instance(Instance* self, char* buf, size_t size);

static const __attribute__((unused)) ObjectVTable instance_vtable;


#endif //JOKER_INSTANCE_H
