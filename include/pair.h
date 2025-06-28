//
// Created by Kilig on 2024/12/13.
//
#pragma once

#ifndef JOKER_PAIR_H
#define JOKER_PAIR_H

#include "common.h"
#include "object.h"

#define macro_is_pair(value)		            is_obj_type(value, OBJ_PAIR)
#define macro_as_pair(value)					((Pair*)macro_as_obj(value))
#define macro_as_pair_from_obj(obj)				((Pair*)obj)


typedef struct Pair {
    Object base;
    Value first;
    Value second;
} Pair;


Pair *new_pair(VirtualMachine *vm, Value first, Value second);
void free_pair(Pair *self);
bool pair_equal(Pair *left, Pair *right);
void print_pair(Pair *self);
int snprintf_pair(Pair *self, char *buf, size_t size);

#endif //JOKER_PAIR_H
