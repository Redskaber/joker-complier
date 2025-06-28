//
// Created by Kilig on 2024/11/21.
//
#pragma once

#ifndef JOKER_CHUNK_H
#define JOKER_CHUNK_H
#include "common.h"
#include "value.h"

/* Error */

/*
 * OpCode: 虚拟机操作码定义
 * 注释格式：编号 位宽（功能说明）
 */
typedef enum OpCode {
    // 基础栈操作
    op_pop,                 // 01     8 bit(pop)
    op_dup,                 // 02     8 bit(dup)

    // 常量操作
    op_constant,            // 03    16 bit(constant + offset[8bit])
    op_constant_long,       // 04    24 bit(constant_long + offset[high 8 bit, low 8 bit])

    // 值操作
    op_value,               // 05     8 bit(value)
    op_none,                // 06     8 bit
    op_true,                // 07     8 bit
    op_false,               // 08     8 bit

    // 运算操作
    op_not,                 // 09     8 bit(unary!)
    op_negate,              // 10     8 bit(unary(-))
    op_equal,               // 11     8 bit(==)
    op_not_equal,           // 12     8 bit(!=)
    op_less,                // 13     8 bit(<)
    op_less_equal,          // 14     8 bit(<=)
    op_greater,             // 15     8 bit(>)
    op_greater_equal,       // 16     8 bit(>=)
    op_add,                 // 17     8 bit(+)
    op_subtract,            // 18     8 bit(-)
    op_multiply,            // 19     8 bit(*)
    op_divide,              // 20     8 bit(/)
    op_mod,                 // 21     8 bit(%)

    // 位运算
    op_bw_and,              // 21     8 bit(&)
    op_bw_or,               // 22     8 bit(|)
    op_bw_xor,              // 23     8 bit(^)
    op_bw_sl,               // 24     8 bit(<<)
    op_bw_sr,               // 25     8 bit(>>)
    op_bw_not,              // 26     8 bit(~)

    // 变量作用域操作
    op_define_global,       // 27    16 bit(define_global + index)
    op_get_global,          // 28    16 bit(get_global + index)
    op_set_global,          // 29    16 bit(set_global + index)
    op_get_local,           // 30    16 bit(get_local + index)
    op_set_local,           // 31    16 bit(set_local + index)
    op_get_upvalue,         // 32    16 bit(get_upvalue + index)
    op_set_upvalue,         // 33    16 bit(set_upvalue + index)

    // 面向对象操作
    op_get_property,        // 34    16 bit(get_property + index)
    op_set_property,        // 35    16 bit(set_property + index)
    op_get_super,           // 36    16 bit(get_super + index)
    op_get_layer_property,  // 37    16 bit(get_layer_property + index)
    op_get_type,            // 38    16 bit(get_type + index)

    // 流程控制
    op_jump_if_false,       // 39    24 bit(jump_if_false + offset)
    op_jump_if_neq,         // 40    24 bit(jump_if_neq + offset)
    op_jump,                // 41    24 bit(jump + offset)
    op_loop,                // 42    24 bit(loop + offset)
    op_call,                // 43    16 bit(call + offset)

    // 闭包操作
    op_closure,             // 44    16 bit(closure + offset)
    op_close_upvalue,       // 45     8 bit(close_upvalue)

    // 系统操作
    op_print,               // 46     8 bit(print)
    op_return,              // 47     8 bit
    op_break,               // 48    24 bit(break + offset)
    op_continue,            // 49    24 bit(continue + offset)
    op_match,               // 50    16 bit(match + offset)

    // 类型与结构
    op_class,               // 51    16 bit(class + index)
    op_method,              // 52    16 bit(method + index)
    op_inherit,             // 53     8 bit(inherit)
    op_invoke,              // 54    24 bit(invoke + callee_index + arg_count)
    op_super_invoke,        // 55    24 bit(super_invoke + callee_index + arg_count)
    op_struct,              // 56    16 bit(struct + index)
    op_member,              // 57    16 bit(member + index)
    op_struct_inherit,      // 58     8 bit(struct_inherit)

    // 枚举操作
    op_enum,                // 59    16 bit(enum + index)
    op_enum_define_member,  // 60    16 bit(enum_define_member + index)
    op_enum_get_member,     // 61    16 bit(enum_get_member + index)
    op_enum_member_bind,    // 62    16 bit(enum_member_bind + arg_count + store_indexes)
    op_enum_member_match,   // 63     8 bit(member_match)

    // 特殊操作
    op_layer_property_call, // 64    24 bit(layer_property_call + property_index + arg_count)
    op_vector_new,          // 65    16 bit(vector_init + element_count)
    op_vector_set,          // 66    24 bit(vector_set + index + set_value)
    op_vector_get,          // 67    16 bit(vector_get + index)
    OP_COUNT,               //       op count
} OpCode;

/*
 * RLE行号压缩结构
 * 用于高效存储代码行号信息
 */
typedef struct RleLine {
    line_t line;    // 原始行号
    int count;      // 连续出现次数
} RleLine;

typedef struct RleLines {
    int count;      // 当前元素数量
    int capacity;   // 分配容量
    RleLine* lines; // 动态数组
} RleLines;

// RLE行号操作函数
void init_rle_lines(RleLines* lines);
void free_rle_lines(VirtualMachine* vm, RleLines* rle_lines);
line_t get_rle_line(RleLines* lines, index_t code_count);

/*
 * Chunk: 字节码块
 * 包含指令序列、行号信息和常量池
 */
typedef struct Chunk {
    VirtualMachine *vm;        // 关联的虚拟机实例
    int count;                 // 当前指令数量
    int capacity;              // 分配的指令容量
    uint8_t* code;             // 指令数组（操作码 | 操作数）
    RleLines lines;            // RLE压缩的行号信息
    Values constants;          // 常量池
} Chunk;

// Chunk操作函数
void init_chunk(Chunk* chunk, VirtualMachine* vm);
void free_chunk(Chunk* chunk);
void write_chunk(Chunk* chunk, uint8_t code, line_t line);
index_t add_constant(Chunk* chunk, Value value);
void write_constant(Chunk* chunk, Value value, line_t line);

#endif //JOKER_CHUNK_H
