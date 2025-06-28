//
// Created by Kilig on 2024/11/21.
//
#pragma once

#ifndef JOKER_STRING__H
#define JOKER_STRING__H
#include "common.h"
#include "object.h"
#include "hashmap.h"
#include "vm.h"

#define macro_is_string(value)  is_obj_type(value, OBJ_STRING)

#define macro_as_string(value) ((String*)macro_as_obj(value))
#define macro_as_cstring(value) ((const char*)macro_as_string(value)->chars)

#define macro_as_string_from_obj(obj) ((String*)obj)
#define macro_as_string_from_value(value) ((String*)macro_as_obj_ptr(value))
#define macro_as_cstring_from_obj(obj) (macro_as_string_from_obj(obj)->chars)

#define macro_stored_string(destValuePtr, string) \
    do {                                          \
        destValuePtr->type  = VAL_OBJECT;         \
        destValuePtr->as.object = (Object*)string;\
    } while(0)

/* flexible array members( 灵活数组成员):
*     1. flexible array member is the last member of a struct.
*     2. flexible array member is declared with the keyword "flexible array member" (e.g. char chars[];).
*     3. flexible array member can be accessed using the pointer to the struct (e.g. chars).
*     4. flexible array member can be resized dynamically.
*/

typedef struct String {
	Object base;
	uint32_t hash;   // hash value of string (string hash)
	int length;
	char chars[];	// flexible array member: from char* need double pointer to char[].
} String;

String* __attribute__((unused)) new_string_uninterned(VirtualMachine *vm, const char* chars, int length);
String* new_string(VirtualMachine *vm, const char* chars, int length);
void free_string(String* string);
String* concat_string_uninterned(String* left, String* right);
String* concat_string(HashMap* interned_pool, String* left, String* right);
bool string_equal(String* left, String* right);
bool __attribute__((unused)) cstring_equal(const char* left, const char* right);
const char* as_cstring(String* string);
void print_string(String* string);
int snprintf_string(String* string, char* buf, size_t size);
String* number_to_string(VirtualMachine* vm, Value* number);



// TODO::User<String>
/*===========================*/
// string virtual table
// used build in type String.
// used in vm.c
/*===========================*/
/* next part string virtual table */

static InterpretResult string_eq(Value* left, Value* right){
    if (macro_is_string(*left) && macro_is_string(*right)) {
        String* left_string = macro_as_string_from_value(left);
        String* right_string = macro_as_string_from_value(right);
        bool is_eq = string_equal(left_string, right_string);
        macro_set_bool(left, is_eq);
        return interpret_ok;
    }
    return interpret_runtime_error;
};

static InterpretResult string_neq(Value* left, Value* right){
    InterpretResult result = string_eq(left, right);
    macro_set_bool(left, !macro_as_bool(*left));
    return result;
}

static InterpretResult string_add(Value* left, Value* right) {
    String* left_string = macro_as_string_from_value(left);
    String* right_string = macro_as_string_from_value(right);
    String* result = concat_string(&left_string->base.vm->strings, left_string, right_string);
    if(!result) return interpret_runtime_error;
    macro_stored_string(left, result);
    return interpret_ok;
}


static const __attribute__((unused)) ObjectVTable string_vtable = {
        .type_name = "String",
        .binary_operators = {
                [EQ]  = (BinaryOpHandler)string_eq,
                [NEQ] = (BinaryOpHandler)string_neq,
                [ADD] = (BinaryOpHandler)string_add,
        },
        .unary_operators = {}
};


#endif //JOKER_STRING__H
