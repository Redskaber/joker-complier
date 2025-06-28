//
// Created by Kilig on 2024/11/21.
//
#pragma once

#ifndef JOKER_COMPILER_H
#define JOKER_COMPILER_H
#include "common.h"
#include "fn.h"
#include "vm.h"
#include "upvalue.h"
#include "token.h"

/* local variable */
typedef struct LocalVariable {
	Token name;
	int depth;
	bool is_captured;
} LocalVariable;

/*
 * Loop {       <- outer start
 *      Loop {  <- inner start
 *
 *      }       <- inner end
 * }            <- outer end
 */
typedef struct Loop {
    int start;                      // 循环开始的位置
    int end;                        // break 指令的跳转位置
    struct Loop* enclosing;         // 包围的循环信息
} Loop;
void begin_loop(Compiler* self, Loop *curr_loop, int start);
void end_loop(Compiler* self);

/*
 *  Match expr {    <- start
 *      pattern1 => {},         --
 *      pattern2 => {},          | (jump)
 *  }               <- end       ↓
 */
typedef struct Match {
    int start;
    int end;
    int stack[uint8_count];         // pattern store codes pos
    int stack_count;                // pattern index in stack
    bool has_default;
} Match;


typedef struct Compiler {
    VirtualMachine *vm;
	struct Compiler* enclosing;                     // enclosing compiler
	LocalVariable locals[uint8_count];              // TODO: optimize
	int local_count;                                // function local variable count range [0, 127]

	int scope_depth;                                // compiler scope depth
	Fn* fn;                                         // function top-level function: e.g. main()
	FnType fn_type;                                 // function type
	upvalue_info_t upvalues[upvalue_index_count];   // function used local variable upvalue info list

    Loop* loop;                                     // loop info
} Compiler;

Fn* compile(VirtualMachine* vm, const char* source);

void init_compiler(Compiler* self, VirtualMachine *vm, FnType type);
void free_compiler(Compiler* self);

Chunk* curr_chunk(Compiler* self);
void compile_begin_scope(Compiler* self);
void compile_end_scope(Compiler* self);

void print_locals(Compiler* self);

#endif //JOKER_COMPILER_H
