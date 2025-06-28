//
// Created by Kilig on 2024/12/11.
//
#pragma once

#ifndef JOKER_VEC_H
#define JOKER_VEC_H
#include "common.h"
#include "object.h"
#include "string_.h"
#include "instance.h"

#define macro_is_vec(value)        is_obj_type(value, OBJ_VEC)
#define macro_as_vec(value)        ((Vec*)macro_as_obj(value))
#define macro_as_vec_from_obj(obj) ((Vec*)(obj))
#define macro_as_vec_from_value_ptr(value_ptr) ((Vec*)(macro_as_obj_ptr(value_ptr)))
#define macro_vec_to_obj(vec)      ((Object*)(vec))

//typedef struct Vec_base {
//    Value* start;
//    Value* finish;
//    Value* end;
//} Vec_base;

typedef struct Vec {
    Object base;
    Value* start;
    Value* finish;
    Value* end;
} Vec;

Vec* new_vec(VirtualMachine* vm);
Vec* new_vec_with_capacity(VirtualMachine* vm, size_t capacity);
void free_vec(Vec* vec);
size_t vec_capacity(Vec* vec);
size_t vec_len(Vec* vec);
void vec_push(Vec* vec, Value element);
Value vec_pop(Vec* vec);
Value vec_first(Vec* vec);
Value vec_last(Vec* vec);
Value vec_get(Vec* vec, size_t index);                // vec[index]
void vec_set(Vec* vec, size_t index, Value element);  // vec[index] = value
void vec_insert(Vec* vec, size_t index, Value element);
void vec_remove(Vec* vec, size_t index);
void vec_extend(Vec* vec, Vec* other);
void vec_clear(Vec* vec);
void vec_reverse(Vec* vec);
bool vec_is_empty(Vec* vec);
void print_vec(Vec* vec);
int snprintf_vec(Vec* vec, char* buf, size_t size);

bool vec_equal(Vec* left, Vec* right);





/*===============================================================================*/
// Vector VTable
// result stored left param
/*===============================================================================*/


static InterpretResult vec_add(Value* left, Value* right) {
    Instance* left_inst = macro_as_instance_from_value_ptr(left);
    Instance* right_inst = macro_as_instance_from_value_ptr(right);
    Vec* left_vec = macro_as_vec(hashmap_get(
            &left_inst->fields,
            new_string(left_inst->base.vm, "_data", 5)
    ));
    Vec* right_vec = macro_as_vec(hashmap_get(
            &right_inst->fields,
            new_string(right_inst->base.vm, "_data", 5)
    ));

    Vec* result = new_vec_with_capacity(
        left_inst->base.vm,
        vec_len(left_vec) + vec_len(right_vec)
    );

    vec_extend(result, left_vec);
    vec_extend(result, right_vec);

    macro_set_obj(left, macro_vec_to_obj(result));
    return interpret_ok;
}

static InterpretResult vec_eq(Value* left, Value* right) {
    Instance* left_inst = macro_as_instance_from_value_ptr(left);
    Instance* right_inst = macro_as_instance_from_value_ptr(right);
    Vec* left_vec = macro_as_vec(hashmap_get(
            &left_inst->fields,
            new_string(left_inst->base.vm, "_data", 5)
    ));
    Vec* right_vec = macro_as_vec(hashmap_get(
            &right_inst->fields,
            new_string(right_inst->base.vm, "_data", 5)
    ));

    macro_set_bool(left, vec_equal(left_vec, right_vec));
    return interpret_ok;
}
static InterpretResult vec_neq(Value* left, Value* right) {
    vec_eq(left, right);
    macro_set_bool(left, !macro_as_bool_ptr(left));
    return interpret_ok;
}

static const __attribute__((unused)) ObjectVTable vec_vtable = {
        .type_name = "Vec",
        .binary_operators = {
                [MACRO_BINARY_INDEX(ADD)] = (BinaryOpHandler)vec_add,
                [MACRO_BINARY_INDEX(EQ)]  = (BinaryOpHandler)vec_eq,
                [MACRO_BINARY_INDEX(NEQ)] = (BinaryOpHandler)vec_neq,
        },
        .unary_operators = {}
};

#endif //JOKER_VEC_H


