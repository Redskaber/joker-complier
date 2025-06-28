//
// Created by Kilig on 2024/11/21.
//
#pragma once

#ifndef JOKER_OPTION_H
#define JOKER_OPTION_H
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "value.h"

typedef enum OptionState {
	NoneState = 0,
	SomeState = 1,
} OptionState;

typedef struct Option_ {
	OptionState state;
} Option_;

typedef struct Some_ {
	Option_ base;
	Value value;
} Some_;

Option_* create_some(VirtualMachine *vm, Value value);
Option_* create_none(VirtualMachine *vm);
void free_option(VirtualMachine *vm, Option_* option);
void print_option(Option_* option);
void printf_option(Option_* option);

#define Option(T) Option_*

#define Some(value) create_some(value)
#define None create_none()

#define is_some(option) ((option)->state == SomeState)
#define is_none(option) ((option)->state == NoneState)
#define as_some(option) ((Some_*)option)

#define unwrap(option) (((Some_*)option)->value)
#define unwrap_or(option, default_value) (is_some(option)? unwrap(option) : (default_value))
#define unwrap_or_else(option, default_func) (is_some(option)? unwrap(option) : (default_func)())
#define expect(option, message) (is_some(option)? unwrap(option) : (panic(message), (Value){0}))

#endif //JOKER_OPTION_H
