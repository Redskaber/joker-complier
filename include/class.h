//
// Created by Kilig on 2024/11/30.
//
#pragma once

#ifndef JOKER_CLASS_H
#define JOKER_CLASS_H
#include "common.h"
#include "hashmap.h"
#include "object.h"

#define macro_is_class(value)            is_obj_type(value, OBJ_CLASS)
#define macro_as_class(value)        ((Class*)macro_as_obj(value))
#define macro_as_class_from_obj(object)  ((Class*)object)
#define macro_as_class_from_vptr(value_ptr)  macro_as_class_from_obj(macro_as_obj_ptr(value_ptr))


typedef struct Class {
    Object base;
    String* name;
    HashMap methods;
} Class;

Class* new_class(VirtualMachine *vm, String* name);
bool class_equal(Class* left, Class* right);
void free_class(Class* self);
void print_class(Class* self);
int snprintf_class(Class* self, char* buf, size_t size);

#endif //JOKER_CLASS_H
