//
// Created by Kilig on 2025/4/2.
//
#pragma once

#ifndef JOKER_TYPE_REGISTER_H
#define JOKER_TYPE_REGISTER_H
#include "common.h"

void type_register(VirtualMachine *vm,
                   const char* type_name,
                   const ObjectVTable* object_vtable,
                   const FnMapper(FnName, FnPtr) methods[][2]);
Class* type_find(VirtualMachine* vm, const char* type_name);
bool type_exists(VirtualMachine* vm, const char* type_name);
void type_dump_registered(VirtualMachine* vm);


#endif //JOKER_TYPE_REGISTER_H
