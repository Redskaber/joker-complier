//
// Created by Kilig on 2024/11/21.
//
#pragma once

#ifndef JOKER_NATIVE_H
#define JOKER_NATIVE_H
#include "common.h"
#include "object.h"

/* Native function type */
typedef Value(*NativeFnPtr)(VirtualMachine* vm, int arg_count, Value* args);

typedef struct Native {
	Object base;
	NativeFnPtr fn;
    bool is_builtin_method;
} Native;

Native* new_native(VirtualMachine* vm, NativeFnPtr fn, bool is_builtin_method);
void free_native(Native* self);
bool native_equal(Native* left, Native* right);
void print_native(Native* self);
int snprintf_native(Native* self, char* buf, size_t size);

#endif //JOKER_NATIVE_H
