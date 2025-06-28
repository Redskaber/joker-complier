//
// Created by Kilig on 2024/11/21.
//
/*
* debug.c
* Bytecode Virtual Machine - Debugging functions
*
* This file contains the implementation of the debugging functions used by the
* bytecode virtual machine.
*/

#include <stdio.h>

#include "error.h"
#include "debug.h"
#include "value.h"
#include "Fn.h"
#include "chunk.h"

// Forward declarations of helper functions
static int simple_instruction(const char* name, int offset);
static int constant_instruction(const char* name, Chunk* chunk, int offset);
static int constant_long_instruction(const char* name, Chunk* chunk, int offset);
static int byte_instruction(const char* name, Chunk* chunk, int offset);
static int jump_instruction(const char* name, int sign, Chunk* chunk, int offset);
static int invoke_instruction(const char* name, Chunk* chunk, int offset);
static int args_instruction(const char* name, Chunk* chunk, int offset);


void disassemble_chunk(Chunk* chunk, const char* name) {
	printf("== %s ==\n", name);
	for (int offset = 0; offset < chunk->count;) {
		// return next instruction offset
		offset = disassemble_instruction(chunk, offset);
	}
}

int disassemble_instruction(Chunk* chunk, int offset) {
	printf("%04d ", offset);

	// print line number (if changed)
	line_t current_line = get_rle_line(&chunk->lines, offset);
	if (offset > 0 && current_line == get_rle_line(&chunk->lines, offset - 1)) {
		printf("   | ");
	}
	else {
		printf("%4d ", current_line);
	}

	// print instruction
	uint8_t instruction = chunk->code[offset];
	switch (instruction) {
    case op_pop:    return simple_instruction("op_pop", offset);
    case op_dup:    return simple_instruction("op_dup", offset);
	case op_constant:return constant_instruction("op_constant", chunk, offset);
	case op_constant_long:return constant_long_instruction("op_constant_long", chunk, offset);

    case op_value:  return simple_instruction("op_value", offset);
    case op_none:   return simple_instruction("op_none", offset);
	case op_true:   return simple_instruction("op_true", offset);
	case op_false:  return simple_instruction("op_false", offset);

	case op_not:      return simple_instruction("op_not", offset);
	case op_negate:   return simple_instruction("op_negate", offset);
	case op_equal:    return simple_instruction("op_equal", offset);
	case op_not_equal:return simple_instruction("op_not_equal", offset);
	case op_less:     return simple_instruction("op_less", offset);
	case op_greater:  return simple_instruction("op_greater", offset);
	case op_greater_equal: return simple_instruction("op_greater_equal", offset);
	case op_add:      return simple_instruction("op_add", offset);
	case op_subtract: return simple_instruction("op_subtract", offset);
	case op_multiply: return simple_instruction("op_multiply", offset);
	case op_divide:   return simple_instruction("op_divide", offset);

    case op_define_global:return constant_instruction("op_define_global", chunk, offset);
	case op_get_global:   return constant_instruction("op_get_global", chunk, offset);
	case op_set_global:   return constant_instruction("op_set_global", chunk, offset);
	case op_get_local:    return byte_instruction("op_get_local", chunk, offset);
	case op_set_local:    return byte_instruction("op_set_local", chunk, offset);
	case op_get_upvalue:  return byte_instruction("op_get_upvalue", chunk, offset);
	case op_set_upvalue:  return byte_instruction("op_set_upvalue", chunk, offset);
    case op_get_property: return constant_instruction("op_get_property", chunk, offset);
    case op_set_property: return constant_instruction("op_set_property", chunk, offset);
    case op_get_super:    return constant_instruction("op_get_super", chunk, offset);
    case op_get_layer_property: return constant_instruction("op_get_layer_property", chunk, offset);

	case op_jump_if_false:return jump_instruction("op_jump_if_false", 1, chunk, offset);
    case op_jump_if_neq:  return jump_instruction("op_jump_if_neq", 1, chunk, offset);
	case op_jump: return jump_instruction("op_jump", 1, chunk, offset);
	case op_loop: return jump_instruction("op_loop", -1, chunk, offset);
	case op_call: return byte_instruction("op_call", chunk, offset);

    case op_closure: {
		offset++;
		uint8_t constant = chunk->code[offset++];
		printf("%-16s %4d ", "op_closure", constant);
		print_value(chunk->constants.values[constant]);
		printf("\n");

		Fn* fn = macro_as_fn(chunk->constants.values[constant]);
		for (int i = 0; i < fn->upvalue_count; i++) {
			bool is_local = chunk->code[offset] & 0x80;
			uint8_t index = chunk->code[offset] & 0x7F;
			printf("%04d      |                     %s %d\n",
				offset, is_local ? "local" : "upvalue", index);
			offset++;
		}
		return offset;
	}
	case op_close_upvalue:return simple_instruction("op_close_upvalue", offset);

    case op_print: return simple_instruction("op_print", offset);
	case op_return:return simple_instruction("op_return", offset);
    case op_break: return jump_instruction("op_break", 1, chunk, offset);
    case op_continue:return jump_instruction("op_continue", -1, chunk, offset);
    case op_match: return jump_instruction("op_match", 1, chunk, offset);
    case op_class: return constant_instruction("op_class", chunk, offset);
    case op_method:return constant_instruction("op_method", chunk, offset);
    case op_inherit:return simple_instruction("op_inherit", offset);
    case op_invoke:return invoke_instruction("op_invoke", chunk, offset);
    case op_super_invoke:return invoke_instruction("op_super_invoke", chunk, offset);
    case op_struct:return constant_instruction("op_struct", chunk, offset);
    case op_member:return constant_instruction("op_member", chunk, offset);
    case op_struct_inherit:return simple_instruction("op_struct_inherit", offset);
    case op_enum:  return constant_instruction("op_enum", chunk, offset);
    case op_enum_define_member:return constant_instruction("op_enum_define_member", chunk, offset);
    case op_enum_get_member:return constant_instruction("op_enum_get_member", chunk, offset);
    case op_enum_member_bind:return args_instruction("op_enum_member_bind", chunk, offset);
    case op_enum_member_match:return simple_instruction("op_enum_member_match", offset);
    case op_layer_property_call:return invoke_instruction("op_layer_property_call", chunk, offset);

    case op_vector_new:return constant_instruction("op_vector_new", chunk, offset);
    case op_vector_get:return simple_instruction("op_vector_get", offset);
    case op_vector_set: return simple_instruction("op_vector_set", offset);
	default:
        warning("{WAINING} [debug::disassemble_instruction] unknown opcode %d\n", instruction);
        return offset + 1;
	}
}

static int simple_instruction(const char* name, int offset) {
	printf("%-16s\n", name);
	return offset + 1;
}

static int constant_instruction(const char* name, Chunk* chunk, int offset) {
	uint8_t constant_offset = chunk->code[offset + 1];
	printf("%-16s %4d '", name, constant_offset);
	print_value(chunk->constants.values[constant_offset]);
	printf("'\n");
	return offset + 2;
}

static int constant_long_instruction(const char* name, Chunk* chunk, int offset) {
	uint16_t constant_offset = (uint16_t)(chunk->code[offset + 1] << 8) | chunk->code[offset + 2];
	printf("%-16s %4d '", name, constant_offset);
	print_value(chunk->constants.values[constant_offset]);
	printf("'\n");
	return offset + 3;
}

static int byte_instruction(const char* name, Chunk* chunk, int offset) {
	uint8_t slot = chunk->code[offset + 1];
	printf("%-16s %4d\n", name, slot);
	return offset + 2;
}

static int jump_instruction(const char* name, int sign, Chunk* chunk, int offset) {
	uint16_t jump_offset = (uint16_t)(chunk->code[offset + 1] << 8) | chunk->code[offset + 2];
	printf("%-16s %4d -> %d\n", name, sign * jump_offset, offset + 3 + sign * jump_offset);
	return offset + 3;
}

static int invoke_instruction(const char* name, Chunk* chunk, int offset) {
	uint8_t constant = chunk->code[offset + 1];
	uint8_t arg_count = chunk->code[offset + 2];
	printf("%-16s (%d args) %4d '", name, arg_count, constant);
	print_value(chunk->constants.values[constant]);
	printf("'\n");
	return offset + 3;
}

static int args_instruction(const char* name, Chunk* chunk, int offset) {
	uint8_t bind_count = chunk->code[offset + 1];
	printf("%-16s (%d args)\n", name, bind_count);
    for (int i = 0; i < bind_count; i++) {
        printf("%04d      |                     %d    ", offset, chunk->code[offset + 2 + i]);
        printf_value(chunk->constants.values[chunk->code[offset + 2 + i]]);
    }
	return offset + 2 + bind_count;
}
