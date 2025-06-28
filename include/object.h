//
// Created by Kilig on 2024/11/21.
//
#pragma once

#ifndef JOKER_OBJ_H
#define JOKER_OBJ_H
#include "common.h"
#include "value.h"

#define macro_obj_type(value)			(macro_as_obj(value)->type)
#define macro_obj_ptr_type(value_ptr)	(macro_as_obj_ptr(value_ptr)->type)
// value ptr -> object ptr
#define macro_into_object(value)        ((Object*)(value))


/* Object type */
typedef enum ObjectType {
	OBJ_STRING,
	OBJ_FN,
	OBJ_NATIVE,
	OBJ_CLOSURE,
	OBJ_UPVALUE,
    OBJ_CLASS,
    OBJ_INSTANCE,
    OBJ_BOUND_METHOD,
    OBJ_STRUCT,
    OBJ_PAIR,
    OBJ_VEC,
    OBJ_ENUM,
    OBJ_ENUM_INSTANCE,
    OBJ_TYPE,
} ObjectType;


/* Object virtual Table impl */
typedef InterpretResult (*BinaryOpHandler)(Value* left, Value* right);
typedef InterpretResult (*UnaryOpHandler)(Value* operand);

typedef struct ObjectVTable {
    BinaryOpHandler binary_operators[BINARY_COUNT];                       // binary operator handlers array
    UnaryOpHandler  unary_operators[OPERATOR_COUNT - BINARY_COUNT - 1];   // unary operator handlers array
    const char* type_name;                                                // type name for dbg
} ObjectVTable;



static InterpretResult default_add(Value* left, Value* right) {
    (void)left;
    (void)right;
    return interpret_runtime_error;
}
static InterpretResult default_sub(Value* left, Value* right) {
    (void)left;
    (void)right;
    return interpret_runtime_error;
}
static InterpretResult default_mul(Value* left, Value* right) {
    (void)left;
    (void)right;
    return interpret_runtime_error;
}
static InterpretResult default_div(Value* left, Value* right) {
    (void)left;
    (void)right;
    return interpret_runtime_error;
}
static InterpretResult default_mod(Value* left, Value* right) {
    (void)left;
    (void)right;
    return interpret_runtime_error;
}
static InterpretResult default_pow(Value* left, Value* right) {
    (void)left;
    (void)right;
    return interpret_runtime_error;
}
static InterpretResult default_eq(Value* left, Value* right) {
    (void)left;
    (void)right;
    return interpret_runtime_error;
}
static InterpretResult default_neq(Value* left, Value* right) {
    (void)left;
    (void)right;
    return interpret_runtime_error;
}
static InterpretResult default_gt(Value* left, Value* right) {
    (void)left;
    (void)right;
    return interpret_runtime_error;
}
static InterpretResult default_lt(Value* left, Value* right) {
    (void)left;
    (void)right;
    return interpret_runtime_error;
}
static InterpretResult default_gte(Value* left, Value* right) {
    (void)left;
    (void)right;
    return interpret_runtime_error;
}
static InterpretResult default_lte(Value* left, Value* right) {
    (void)left;
    (void)right;
    return interpret_runtime_error;
}
static InterpretResult default_and(Value* left, Value* right) {
    (void)left;
    (void)right;
    return interpret_runtime_error;
}
static InterpretResult default_or(Value* left, Value* right) {
    (void)left;
    (void)right;
    return interpret_runtime_error;
}
static InterpretResult default_xor(Value* left, Value* right) {
    (void)left;
    (void)right;
    return interpret_runtime_error;
}
static InterpretResult default_neg(Value* left, Value* right) {
    (void)left;
    (void)right;
    return interpret_runtime_error;
}
static InterpretResult default_assign(Value* left, Value* right) {
    (void)left;
    (void)right;
    return interpret_runtime_error;
}
static InterpretResult default_shl(Value* left, Value* right) {
    (void)left;
    (void)right;
    return interpret_runtime_error;
}
static InterpretResult default_shr(Value* left, Value* right) {
    (void)left;
    (void)right;
    return interpret_runtime_error;
}
static InterpretResult default_bit_and(Value* left, Value* right) {
    (void)left;
    (void)right;
    return interpret_runtime_error;
}
static InterpretResult default_bit_or(Value* left, Value* right) {
    (void)left;
    (void)right;
    return interpret_runtime_error;
}
static InterpretResult default_bit_xor(Value* left, Value* right) {
    (void)left;
    (void)right;
    return interpret_runtime_error;
}

static InterpretResult default_not(Value* operand) {
    (void)operand;
    return interpret_runtime_error;
}
static InterpretResult default_bit_not(Value* operand) {
    (void)operand;
    return interpret_runtime_error;
}


/* default object virtual table */
static const ObjectVTable default_object_vtable = {
    .type_name = "",
    .binary_operators = {
        [MACRO_BINARY_INDEX(ADD)]   = (BinaryOpHandler)default_add,
        [MACRO_BINARY_INDEX(SUB)]   = (BinaryOpHandler)default_sub,
        [MACRO_BINARY_INDEX(MUL)]   = (BinaryOpHandler)default_mul,
        [MACRO_BINARY_INDEX(DIV)]   = (BinaryOpHandler)default_div,
        [MACRO_BINARY_INDEX(MOD)]   = (BinaryOpHandler)default_mod,
        [MACRO_BINARY_INDEX(POW)]   = (BinaryOpHandler)default_pow,
        [MACRO_BINARY_INDEX(EQ)]    = (BinaryOpHandler)default_eq,
        [MACRO_BINARY_INDEX(NEQ)]   = (BinaryOpHandler)default_neq,
        [MACRO_BINARY_INDEX(GT)]    = (BinaryOpHandler)default_gt,
        [MACRO_BINARY_INDEX(LT)]    = (BinaryOpHandler)default_lt,
        [MACRO_BINARY_INDEX(GTE)]   = (BinaryOpHandler)default_gte,
        [MACRO_BINARY_INDEX(LTE)]   = (BinaryOpHandler)default_lte,
        [MACRO_BINARY_INDEX(AND)]   = (BinaryOpHandler)default_and,
        [MACRO_BINARY_INDEX(OR)]    = (BinaryOpHandler)default_or,
        [MACRO_BINARY_INDEX(XOR)]   = (BinaryOpHandler)default_xor,
        [MACRO_BINARY_INDEX(NEG)]   = (BinaryOpHandler)default_neg,
        [MACRO_BINARY_INDEX(ASSIGN)] = (BinaryOpHandler)default_assign,
        [MACRO_BINARY_INDEX(SHL)]   = (BinaryOpHandler)default_shl,
        [MACRO_BINARY_INDEX(SHR)]   = (BinaryOpHandler)default_shr,
        [MACRO_BINARY_INDEX(BIT_AND)] = (BinaryOpHandler)default_bit_and,
        [MACRO_BINARY_INDEX(BIT_OR)] = (BinaryOpHandler)default_bit_or,
        [MACRO_BINARY_INDEX(BIT_XOR)] = (BinaryOpHandler)default_bit_xor,
    },
    .unary_operators = {
        [MACRO_UNARY_INDEX(NOT)]    = (UnaryOpHandler)default_not,
        [MACRO_UNARY_INDEX(BIT_NOT)] = (UnaryOpHandler)default_bit_not,
    }
};



/* Object structure store object total of information */
typedef struct Object {
    VirtualMachine *vm;
    const ObjectVTable* vtable;
    ObjectType type;	    // label of an object type
    bool is_marked;
    struct Object* next;
} Object;

Object* allocate_object(VirtualMachine *vm, size_t size, ObjectType type);
bool object_equal(Object* left, Object* right);
void free_object(Object* object);
void free_objects(Object* object);
void print_object(Object* object);
int snprintf_object(Object* object, char* buf, size_t size);


// macro for allocate object
#define macro_allocate_object(vm, type, object_type) \
	(type*)allocate_object(vm, sizeof(type), object_type)

/* Why not just put the body of this function right in the macro?
* It's not a good practice to put a function body inside a macro, because it can cause
* unexpected behavior when the macro is expanded.
* (for example is_string(pop()) will double pop)
*/
static inline bool is_obj_type(Value value, ObjectType type) {
	return macro_is_obj(value) && macro_as_obj(value)->type == type;  // is object and type match
}

#define macro_object_type_string(type)  \
    (type == OBJ_STRING ? "STRING" :    \
    type == OBJ_FN ? "FN" :             \
    type == OBJ_NATIVE ? "NATIVE" :     \
    type == OBJ_CLOSURE ? "CLOSURE" :   \
    type == OBJ_CLASS ? "CLASS":        \
    type == OBJ_UPVALUE ? "UPVALUE" :   \
    type == OBJ_INSTANCE ? "INSTANCE" : \
    type == OBJ_BOUND_METHOD ? "BOUND_METHOD" :     \
    type == OBJ_STRUCT ? "STRUCT" :     \
    type == OBJ_PAIR ? "PAIR" :         \
    type == OBJ_VEC ? "VEC" :           \
    type == OBJ_ENUM ? "ENUM" :         \
    type == OBJ_ENUM_INSTANCE ? "ENUM_INSTANCE" :   \
    type == OBJ_TYPE ? "TYPE" :                     \
    "UNKNOWN")

#endif //JOKER_OBJ_H
