//
// Created by Kilig on 2025/4/2.
//

#ifndef JOKER_NATIVE_VEC_H
#define JOKER_NATIVE_VEC_H
#include "common.h"

extern Value native_vec_new(VirtualMachine* vm, int arg_count, Value* args);
extern Value native_vec_get(VirtualMachine* vm, int arg_count, Value* args);
extern Value native_vec_set(VirtualMachine* vm, int arg_count, Value* args);
extern Value native_vec_push(VirtualMachine* vm, int arg_count, Value* args);
extern Value native_vec_pop(VirtualMachine* vm, int arg_count, Value* args);
extern Value native_vec_insert(VirtualMachine* vm, int arg_count, Value* args);
extern Value native_vec_remove(VirtualMachine* vm, int arg_count, Value* args);
extern Value native_vec_extend(VirtualMachine* vm, int arg_count, Value* args);
extern Value native_vec_clear(VirtualMachine* vm, int arg_count, Value* args);
extern Value native_vec_reverse(VirtualMachine* vm, int arg_count, Value* args);
extern Value native_vec_length(VirtualMachine* vm, int arg_count, Value* args);
extern Value native_vec_first(VirtualMachine* vm, int arg_count, Value* args);
extern Value native_vec_last(VirtualMachine* vm, int arg_count, Value* args);
extern const FnMapper(FnName, FnPtr) vec_export_methods[][2];

#endif //JOKER_NATIVE_VEC_H
