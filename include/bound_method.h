//
// Created by Kilig on 2024/11/30.
//
#pragma once

#ifndef JOKER_BOUND_METHOD_H
#define JOKER_BOUND_METHOD_H

#include "common.h"
#include "object.h"
#include "value.h"


#define macro_is_bound_method(value)            is_obj_type(value, OBJ_BOUND_METHOD)

#define macro_as_bound_method_from_vptr(value_ptr)  ((BoundMethod*)macro_as_obj_ptr(value_ptr))
#define macro_as_bound(value)                       ((BoundMethod*)macro_as_obj(value))
#define macro_as_bound_method_from_obj(object)      ((BoundMethod*)object)


typedef struct BoundMethod {
    Object base;
    Value receiver;
    Closure *method;
} BoundMethod;


BoundMethod* new_bound_method(VirtualMachine *vm, Value receiver, Closure *method);
void free_bound_method(BoundMethod* self);
bool bound_method_equal(BoundMethod* left, BoundMethod* right);
void print_bound_method(BoundMethod* self);
int snprintf_bound_method(BoundMethod* self, char* buf, size_t size);

#endif //JOKER_BOUND_METHOD_H
