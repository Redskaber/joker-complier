//
// Created by Kilig on 2024/11/21.
//
#pragma once

#ifndef JOKER_VM_H
#define JOKER_VM_H
#include "common.h"
#include "chunk.h"
#include "native.h"
#include "closure.h"
#include "value.h"
#include "hashmap.h"
#include "gc.h"




#define frames_stack_max 64                // static const int frames_stack_max = 64;
#define constant_stack_max (frames_stack_max * uint8_count)

typedef struct CallFrame {
	Value* slots;                           // vm stack get function base address. (ptr ->slots {Value, Value, ...})
	Closure* closure;                       // the closure of the function
	uint8_t* ip;                            // func self instruction pointer. (return address: func ip execute over call caller ip continue execute)
} CallFrame;
/*
fn func1() {
	func2();
}
fn func2() {
	func3();
}
fn func3() {
	return 1;
}
call func1()
	-> execute func1->ip     (over func1)   ← continue execute up_func->ip
	-> execute func2->ip     (over func2)   | continue execute func1->ip
	-> execute func3->ip  ---(over func3)---> continue execute func2->ip
*/

typedef struct VirtualMachine {
    TokenList *tokens;

	CallFrame frames[frames_stack_max];     // the func stack: {vm->ip} goto {vm->frames[index]->ip}
	int frame_count;                        // the call stack count
	Value stack[constant_stack_max];        // the stack
	Value* stack_top;                       // top of the stack

	HashMap strings;                        // string constants
	HashMap globals;                        // global variables
    Object *objects;                        // object list
	Upvalue* open_upv_ptr;                  // upvalue pointer header node
	Compiler* compiler;                     // the compiler
    ClassCompiler* class_compiler;          // class compiler

    Gc gc;                                  // garbage collector

#if debug_enable_allocator
    Allocator* allocator;                   // allocator
#endif
    String* init_string;                    // the init string

    HashMap types;                          // type
} VirtualMachine;

void init_virtual_machine(VirtualMachine* self);
void free_virtual_machine(VirtualMachine* self);

Compiler* replace_compiler(VirtualMachine* self, Compiler* compiler);
InterpretResult interpret(VirtualMachine* self, const char* source);
void runtime_error(VirtualMachine* self, const char* message, ...);

/* Value stack operations */
void push(VirtualMachine* self, Value value);
Value pop(VirtualMachine* self);


/* 指令处理元数据 */
typedef InterpretResult (*OperatorHandler)(VirtualMachine* vm, CallFrame* callFrame);

typedef struct {
    const char* name;           // 指令名称
    uint8_t operand_width;      // 操作数宽度
    OperatorHandler handler;    // 处理函数
} OpMetadata;


/* 指令元数据表 */
static const OpMetadata __attribute__((unused)) op_meta[256];

#endif //JOKER_VM_H
