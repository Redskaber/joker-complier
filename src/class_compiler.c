//
// Created by Kilig on 2024/12/1.
//

#include <stdbool.h>
#include "vm.h"
#include "class_compiler.h"


void into_curr_class_compiler(ClassCompiler * curr, VirtualMachine* vm) {
    curr->has_superclass = false;
    curr->enclosing = vm->class_compiler;
    vm->class_compiler = curr;
}

void set_curr_class_superclass(ClassCompiler* curr, bool has_superclass) {
    curr->has_superclass = has_superclass;
}

void out_curr_class_compiler(VirtualMachine* vm) {
    vm->class_compiler = vm->class_compiler->enclosing;
}
