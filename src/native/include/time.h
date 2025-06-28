//
// Created by Kilig on 2025/3/31.
//
#pragma once

#ifndef JOKER_TIME_H
#define JOKER_TIME_H
#include "common.h"

Value native_clock(VirtualMachine* vm, int arg_count, Value* args);
Value native_time(VirtualMachine* vm, int arg_count, Value* args);
Value native_sleep(VirtualMachine* vm, int arg_count, Value* args);
Value native_date(VirtualMachine* vm, int arg_count, Value* args);
Value native_now(VirtualMachine* vm, int arg_count, Value* args);

#endif //JOKER_TIME_H
