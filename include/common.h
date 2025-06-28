//
// Created by Kilig on 2024/11/21.
//
#pragma once

#ifndef JOKER_COMMON_H
#define JOKER_COMMON_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define JOKER_VERSION "0.1.0"


/* debug depart enable*/
#define enable_depart_debug false

/* debug print keyword */
#define deprecated_print_keyword true


/* TODO: OPTIMIZE */
// #define NAN_BOXING

typedef int index_t;
typedef int line_t;

typedef enum OpCode OpCode;
typedef struct RleLine RleLine;
typedef struct RleLines RleLines;
typedef struct Chunk Chunk;
typedef struct Closure Closure;
typedef struct LocalVariable LocalVariable;
typedef struct Compiler Compiler;
typedef enum FnType FnType;
typedef struct Fn Fn;
typedef struct Entry Entry;
typedef struct HashMap HashMap;
typedef struct Native Native;
typedef enum ObjectType ObjectType;
typedef struct Object Object;
typedef enum Precedence Precedence;
typedef struct Parser Parser;
typedef struct ParseRule ParseRule;
typedef struct Scanner Scanner;
typedef struct String String;
typedef struct Token Token;
typedef struct TokenNode TokenNode;
typedef struct TokenList TokenList;
typedef struct Upvalue Upvalue, *UpvaluePtr, **UpvaluePtrs;
#ifdef NAN_BOXING
typedef uint64_t Value;
#else
typedef struct Value Value;
#endif

typedef struct Values Values;
// typedef enum InterpretResult InterpretResult;
typedef enum InterpretResult {
    interpret_ok,
    interpret_passed,
    interpret_compile_error,
    interpret_runtime_error
} InterpretResult;

typedef struct VirtualMachine VirtualMachine;
typedef struct Allocator Allocator;
typedef struct GarbageCollector Gc;
typedef struct Class Class;
typedef struct Instance Instance;
typedef struct ClassCompiler ClassCompiler;
typedef struct Struct Struct;
typedef struct Member Member;
typedef struct ObjectVTable ObjectVTable;


/* build-in data-struct */
typedef struct Pair Pair;
typedef struct Vec Vec;
typedef struct Enum Enum;
typedef struct EnumInstance EnumInstance;


/* user defined data-struct */
typedef struct Type Type;
typedef Value(*NativeFnPtr)(VirtualMachine* vm, int arg_count, Value* args);

#define FnName const char*
#define FnPtr  void*
#define FnMapper(name, method) void*

typedef InterpretResult (*BinaryOpHandler)(Value* left, Value* right);
typedef InterpretResult (*UnaryOpHandler)(Value* operand);

typedef struct Symbol Symbol;

typedef void(*JokerConsoleFn) (int argc, char *argv[]);
typedef struct JokerConsole JokerConsole;

/* debug parameters */
#define debug_print_code        false       // print bytecode
#define debug_trace_execution   false       // call trace execution
#define debug_print_allocations false       // print allocations
#define debug_stress_gc         false       // stress gc
#define debug_log_gc            false       // log gc

#define debug_enable_allocator  false
#define debug_trace_allocator   false


/* optional struct Option {Some, None} */

#define argument_count_max UINT8_MAX
/* define local variable limit count store max */
#define uint8_count (UINT8_MAX + 1)

/* define exit code for joker */
typedef enum
{
	enum_success        = 0,
	enum_file_error     = 74,
	enum_invalid_arguments = 64,
	enum_compiler_error = 65,
	enum_scanner_error  = 66,
	enum_runtime_error  = 70,
} JokerExitCode;

/* Display error code to user message  */
#define macro_code_to_string(exit_code)                         \
        ( exit_code == enum_success        ? "success"          \
        : exit_code == enum_file_error     ? "file_error"       \
        : exit_code == enum_invalid_arguments  ? "invalid_arguments" \
        : exit_code == enum_compiler_error ? "compiler_error"   \
        : exit_code == enum_scanner_error  ? "scanner_error"    \
        : exit_code == enum_runtime_error  ? "runtime_error"    \
        : "unknown error")

#endif //JOKER_COMMON_H
