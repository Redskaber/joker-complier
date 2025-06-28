//
// Created by Kilig on 2024/11/21.
//

#include "error.h"
#include "memory.h"
#include "chunk.h"
#include "vm.h"


void init_rle_lines(RleLines* rle_line) {
	rle_line->count = 0;
	rle_line->capacity = 0;
	rle_line->lines = NULL;
}

void free_rle_lines(VirtualMachine* vm, RleLines* rle_line) {
	macro_free_array(vm, RleLine, rle_line->lines, rle_line->capacity);
	init_rle_lines(rle_line);
}

/**
 * 根据给定的code_count获取相应的rle_line
 * 此函数通过累加lines中每个line的count，找到第一个使total_count超过或等于code_count的line并返回
 * 如果code_count超出lines中所有line的count总和，则触发panic
 *
 * @param lines RleLines结构体指针，包含多个rle_line
 * @param code_count 指定的代码计数，用于定位到特定的rle_line
 * @return 返回找到的rle_line
 * @note 此函数假设code_count总是有效的，不会超出lines中代码的总范围
 */
line_t get_rle_line(RleLines* lines, index_t code_count) {
    int total_count = 0;
    for (int i = 0; i < lines->count; i++) {
        total_count += lines->lines[i].count;
        if (code_count < total_count) {
            return lines->lines[i].line;
        }
    }
    panic("[ {PANIC} Chunk::get_rle_line] Code count out of range.");
}


// Chunk structure:
void init_chunk(Chunk* chunk, VirtualMachine* vm) {
    chunk->vm = vm;
	chunk->count = 0;
	chunk->capacity = 0;
	chunk->code = NULL;
	init_rle_lines(&chunk->lines);
	init_value_array(&chunk->constants, vm);
}

void free_chunk(Chunk* chunk) {
	macro_free_array(chunk->vm, uint8_t, chunk->code, chunk->capacity);
	free_rle_lines(chunk->vm, &chunk->lines);
	free_value_array(&chunk->constants);
    chunk->vm = NULL;
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->code = NULL;
    chunk->constants.vm = NULL;
    chunk->constants.count = 0;
    chunk->constants.capacity = 0;
    chunk->constants.values = NULL;
    init_rle_lines(&chunk->lines);
}

/*
* It writes the given code and line to the chunk¡¯s code array
* and also updates the RLE lines accordingly.
* op_constant and op_constant_long are encoded as uint8_t and uint16_t respectively.
*/
void write_chunk(Chunk* chunk, uint8_t code, line_t line) {
	if (chunk->capacity < chunk->count + 1) {
		int old_capacity = chunk->capacity;
		chunk->capacity = macro_grow_capacity(old_capacity);
		chunk->code = macro_grow_array(
            chunk->vm,
			uint8_t,
			chunk->code,
			old_capacity,
			chunk->capacity
		);
		chunk->lines.lines = macro_grow_array(
            chunk->vm,
			RleLine,
			chunk->lines.lines,
			old_capacity,
			chunk->capacity
		);
	}

	// add line to RLE lines
	if (chunk->lines.count == 0 || chunk->lines.lines[chunk->lines.count - 1].line != line) {
		if (chunk->lines.capacity < chunk->lines.count + 1) {
			int old_capacity = chunk->lines.capacity;
			chunk->lines.capacity = macro_grow_capacity(old_capacity);
			chunk->lines.lines = macro_grow_array(
                chunk->vm,
				RleLine,
				chunk->lines.lines,
				old_capacity,
				chunk->lines.capacity
			);
		}
		chunk->lines.lines[chunk->lines.count].line = line;
		chunk->lines.lines[chunk->lines.count].count = 1;
		chunk->lines.count++;
	}
	else {
		chunk->lines.lines[chunk->lines.count - 1].count++;
	}

	chunk->code[chunk->count] = code;
	chunk->count++;
}

index_t add_constant(Chunk* chunk, Value value) {
	if (chunk->constants.count >= INT32_MAX) {
		panic("[ {PANIC} Chunk::add_constant] Excepted constant count in uint32_t, Found up overflow.");
	}

    push(chunk->vm, value);
	write_value_array(&chunk->constants, value);
    pop(chunk->vm);

	return chunk->constants.count - 1;  // index of the added constant
}

/*
* It writes the given constant value to the chunk¡¯s code array
* and also updates the RLE lines accordingly.
*
* TODO: what handle op_constant_long? uint16_t?
*  answer: used double call of write_chunk() to write op_constant_long and uint16_t
*/
void write_constant(Chunk* chunk, Value value, line_t line) {
	index_t index = add_constant(chunk, value);
	if (index < UINT8_MAX) {
		write_chunk(chunk, op_constant, line);
		write_chunk(chunk, (uint8_t)index, line);
	}
	else if (index < UINT16_MAX) {
		write_chunk(chunk, op_constant_long, line);
		write_chunk(chunk, (uint8_t)(index >> 8), line);
		write_chunk(chunk, (uint8_t)index, line);
	}
	else {
		panic("[ {PANIC} Chunk::write_constant] Expected index in uint8_t or uint16_t, Found up overflow.");
	}
}
