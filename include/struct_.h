//
// Created by Kilig on 2024/12/9.
//
#pragma once

#ifndef JOKER_STRUCT__H
#define JOKER_STRUCT__H
#include "common.h"
#include "hashmap.h"
#include "object.h"

#define macro_is_struct(value)            is_obj_type(value, OBJ_STRUCT)
#define macro_as_struct(value_ptr)        ((Struct*)macro_as_obj_ptr(value_ptr))
#define macro_as_struct_from_obj(object)  ((Struct*)object)
#define macro_as_struct_from_value(value) macro_as_struct_from_obj(macro_as_obj(value))


typedef struct Struct {
    Object base;
    String* name;
    HashMap fields;
    Vec* names;
    int count;
} Struct;

Struct* new_struct(VirtualMachine *vm, String* name);
void free_struct(Struct* self);
bool struct_equal(Struct* left, Struct* right);
void print_struct(Struct* self);
int snprintf_struct(Struct* self, char* buf, size_t size);

#endif //JOKER_STRUCT__H
