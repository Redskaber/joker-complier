//
// Created by Kilig on 2024/12/1.
//
#pragma once

#ifndef JOKER_CLASS_COMPILER_H
#define JOKER_CLASS_COMPILER_H
#include "common.h"

typedef struct ClassCompiler {
    struct ClassCompiler *enclosing;
    bool has_superclass;
} ClassCompiler;

void into_curr_class_compiler(ClassCompiler * curr, VirtualMachine* vm);
void set_curr_class_superclass(ClassCompiler* curr, bool has_superclass);
void out_curr_class_compiler(VirtualMachine* vm);

#endif //JOKER_CLASS_COMPILER_H
