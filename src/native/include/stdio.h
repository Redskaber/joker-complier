//
// Created by Kilig on 2025/3/31.
//

#ifndef JOKER_STDIO_H
#define JOKER_STDIO_H
#include "common.h"

Value native_print(VirtualMachine* vm, int arg_count, Value* args);
Value native_println(VirtualMachine* vm, int arg_count, Value* args);

#endif //JOKER_STDIO_H
