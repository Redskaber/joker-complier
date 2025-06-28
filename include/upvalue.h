//
// Created by Kilig on 2024/11/21.
//
#pragma once

#ifndef JOKER_UPVALUE_H
#define JOKER_UPVALUE_H
#include <stdint.h>

#include "object.h"

#define upvalue_index_count uint8_count >> 1    // Upvalue count: index range [0, 127] = 128
typedef uint8_t upvalue_info_t;                 // upvalue_info_t: { is_local: 1bit, index: 7bit }

#define macro_is_upvalue(value)	is_obj_type(value, OBJ_UPVALUE)

#define macro_as_upvalue(value)					((Upvalue*)macro_as_obj(value))
#define macro_as_upvalue_from_obj(object)		((Upvalue*)object)



/* upvalue node to link list */
typedef struct Upvalue {
	Object base;
	Value* location;			// pointer stack slot
	Value closed;				// closed store upvalue
	struct Upvalue* next;		// pointer next upvalue
} Upvalue, *UpvaluePtr, **UpvaluePtrs, **UpvaluePtrRef;

upvalue_info_t new_upvalue_info(uint8_t index, bool is_local);
Upvalue* new_upvalue(VirtualMachine *vm, Value* slot);
void free_upvalue(UpvaluePtr self);
bool upvalue_equal(UpvaluePtr a, UpvaluePtr b);
void print_upvalues(UpvaluePtr self);
void print_upvalue(UpvaluePtr self);
Upvalue* capture_upvalue(VirtualMachine *vm, Value* local);
void close_upvalues(UpvaluePtr root, Value* last);
int snprintf_upvalue(UpvaluePtr self, char* buf, size_t size);

#endif //JOKER_UPVALUE_H
