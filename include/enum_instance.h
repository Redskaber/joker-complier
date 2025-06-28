//
// Created by Kilig on 2024/12/14.
//
#pragma once

#ifndef JOKER_ENUM_INSTANCE_H
#define JOKER_ENUM_INSTANCE_H
#include "common.h"
#include "object.h"

#define macro_is_enum_instance(value)                (is_obj_type(value, OBJ_ENUM_INSTANCE))
#define macro_as_enum_instance(value_ptr)            ((EnumInstance*)(macro_as_obj_ptr(value_ptr)))
#define macro_as_enum_instance_from_obj(obj)         ((EnumInstance*)obj)
#define macro_as_enum_instance_from_value(value)     macro_as_enum_instance_from_obj(macro_as_obj(value))


typedef struct EnumInstance {
    Object base;
    Enum* enum_type;
    int32_t index;
    String* name;
    Vec* values;
} EnumInstance;


EnumInstance* new_enum_instance(VirtualMachine *vm, Enum* enum_type, String* name, int32_t index, int capacity);
void print_enum_instance(EnumInstance *self);
void free_enum_instance(EnumInstance *self);
bool enum_instance_equal(EnumInstance *left, EnumInstance *right);
bool enum_values_is_empty(EnumInstance* self);
int snprintf_enum_instance(EnumInstance *self, char *buf, size_t size);

#endif //JOKER_ENUM_INSTANCE_H
