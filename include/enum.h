//
// Created by Kilig on 2024/12/12.
//
#pragma once

#ifndef JOKER_ENUM_H
#define JOKER_ENUM_H

#include "common.h"
#include "object.h"
#include "hashmap.h"

#define macro_is_enum(value)                (is_obj_type(value, OBJ_ENUM))
#define macro_as_enum(value_ptr)            ((Enum*)(macro_as_obj_ptr(value_ptr)))
#define macro_as_enum_from_obj(obj)         ((Enum*)obj)
#define macro_as_enum_from_value(value)     macro_as_enum_from_obj(macro_as_obj(value))

/*
 * enum identifier "{" (member,)? member "}"
 *
 * member: identifier ("(" identifier ")")?
 *
 * enum Color {
 *    R,
 *    B,
 *    G
 * }
 *
 * enum Demo {
 *   Pattern_1(R),
 *   Pattern_2(B),
 *   Pattern_3(G),
 * }
 */
typedef struct Enum {
    Object base;
    String* name;
    HashMap members;
} Enum;


Enum* new_enum(VirtualMachine *vm, String* name);
void print_enum(Enum *self);
void free_enum(Enum *self);
bool enum_equal(Enum *left, Enum *right);
int snprintf_enum(Enum *self, char *buf, size_t size);

#endif //JOKER_ENUM_H
