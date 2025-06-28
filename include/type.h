//
// Created by Kilig on 2024/12/22.
//
#pragma once

#ifndef JOKER_TYPE_H
#define JOKER_TYPE_H

#include "common.h"
#include "object.h"
#include "hashmap.h"
#include "value.h"

#define macro_as_type_from_obj(obj) ((Type*)(obj))

typedef struct Type {
    Object base;
    String* name;
    HashMap methods;
} Type;


Type* new_type(VirtualMachine* vm, String* name);
void free_type(Type* type);
void print_type(Type* type);
bool type_equal(Type* left, Type* right);
int snprintf_type(Type* type, char* buf, size_t size);

#endif //JOKER_TYPE_H
