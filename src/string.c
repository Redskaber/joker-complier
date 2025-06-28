//
// Created by Kilig on 2024/11/21.
//
#include <stdio.h>
#include <string.h>
#include <inttypes.h>  // 添加头文件以使用 PRId64

#include "error.h"
#include "memory.h"
#include "string_.h"
#include "vm.h"


// macro for allocate fixed array
#define macro_allocate_fixed_array(vm, type, elem_type, arr_size, object_type) \
    (type*)allocate_object(vm, (sizeof(type) + arr_size * sizeof(elem_type)), object_type)
// macro for free fixed array
#define macro_free_fixed_array(vm, type, pointer, elem_type, arr_size) \
    reallocate(vm, pointer, (sizeof(type) + arr_size * sizeof(elem_type)), 0)

static uint32_t hash_string(const char* key, int length);

/* flexible array member */
String* __attribute__((unused)) new_string_uninterned(VirtualMachine * vm, const char* chars, int32_t length) {
	if (chars == NULL && length != 0) {
		panic("[ {PANIC} String::new_string_uninterned] Expected non-null string chars, Found null string chars.");
	}
	if (length < 0 || length > INT32_MAX - 1) {
		panic("[ {PANIC} String::new_string_uninterned] Expected string length between 0 and 2147483647, Found invalid string length.");
	}

	/* fixed array member*/
	String* string = macro_allocate_fixed_array(vm, String, char, length + 1, OBJ_STRING);
	string->length = length;
	if (chars != NULL) {
		memcpy_s(string->chars, length, chars, length);
	}
	string->chars[length] = '\0';

	/* Hash string */
	string->hash = hash_string(string->chars, length);
    string->base.vtable = &string_vtable;

	return string;
}

String* new_string(VirtualMachine * vm, const char* chars, int32_t length) {

	if (chars == NULL && length != 0) {
        runtime_error(vm, "Expected non-null string chars, Found null string chars.");
        return NULL;
	}
	if (length < 0 || length > INT32_MAX - 1) {
        runtime_error(vm, "Expected string length between 0 and 2147483647, Found invalid string length.");
        return NULL;
	}

	/* Check if the string is already interned */
	uint32_t hash = hash_string(chars, length);
	String* interned_string = hashmap_find_key(&vm->strings, chars, length, hash);
	if (interned_string != NULL) {
		return interned_string;
	}
	/* Create a new string */
	String* string = macro_allocate_fixed_array(vm, String, char, length + 1, OBJ_STRING);
	string->length = length;
	if (chars != NULL) {
		memcpy_s(string->chars, length, chars, length);
	}

	string->chars[length] = '\0';
	string->hash = hash;
    string->base.vtable = &string_vtable;

	/* Add the new string to the interned pool */
    push(vm, macro_val_from_obj(string));
    hashmap_set(&vm->strings, string, macro_val_null);
    pop(vm);

    return string;
}

void free_string(String* string) {
    if(string != NULL) {
        if (string->length < 0 || string->length > INT32_MAX - 1) {
            runtime_error(string->base.vm,
              "Expected string length in int32 range, Found invalid string length.");
            return;
        }
        macro_free_fixed_array(string->base.vm, String, string, char, string->length + 1);
    }
}

String* concat_string_uninterned(String* left, String* right) {
	if (left == NULL || right == NULL) return NULL;
	if (left->length < 0 || left->length > INT32_MAX - 1) {
        runtime_error(left->base.vm,
          "Expected left string length in int32 range, Found invalid string length.");
        return NULL;
	}
	if (right->length < 0 || right->length > INT32_MAX - 1) {
        runtime_error(right->base.vm,
          "Expected right string length in int32 range, Found invalid string length.");
        return NULL;
	}
	if (left->length + right->length > INT32_MAX - 1) {
        runtime_error(left->base.vm,
          "Expected result string length in int32 range, Found invalid string length.");
        return NULL;
	}

	int new_length = left->length + right->length;
	String* result = macro_allocate_fixed_array(left->base.vm, String, char, new_length + 1, OBJ_STRING);
	if (result == NULL) {
        runtime_error(left->base.vm,
          "Expected non-null memory, Found null memory.");
        return NULL;
	}

	result->length = new_length;
	memset(result->chars, 0, new_length + 1);
	memcpy_s(result->chars, new_length, left->chars, left->length);
	memcpy_s(
        result->chars + left->length,
        new_length - left->length,
        right->chars,
        right->length
     );
	result->chars[new_length] = '\0';

	/* Hash string */
	result->hash = hash_string(result->chars, new_length);
    result->base.vtable = &string_vtable;

	return result;
}

String* concat_string(HashMap* interned_pool, String* left, String* right) {
	String* result = concat_string_uninterned(left, right);
    if(!result) return NULL;

	String* interned_string = hashmap_find_key(
        interned_pool, result->chars, result->length, result->hash);
	if (interned_string != NULL) {
		free_string(result);
		return interned_string;
	}
	hashmap_set(interned_pool, result, macro_val_null);
	return result;
}

/*
* base: return (left->length == right->length && mem cmp(left->chars, right->chars, left->length) == 0)
* because used interned pool, so same string, it points to the same memory address.
*/
bool string_equal(String* left, String* right) {
	return left == right;
}

bool __attribute__((unused)) cstring_equal(const char* left, const char* right) {
	if (left == NULL || right == NULL) return false;
	if (left == right) return true;

    size_t left_length = strlen(left);
    size_t right_length = strlen(right);
	if (left_length != right_length) return false;

	return memcmp(left, right, left_length) == 0;
}

/* Hash string
*  - FNV-1a:
*  hash need: 1.deterministic 2.uniform 3.fast
*/
static uint32_t hash_string(const char* key, int length) {
	uint32_t hash = 2166136261u;
	for (int i = 0; i < length; i++) {
		hash ^= (uint8_t)key[i];  // xor with the next byte, 1 char = 1 byte
		hash *= 16777619u;
	}
	return hash;
}

const char* as_cstring(String* string) {
	return string->chars;
}

void print_string(String* string) {
	printf("%s", string->chars);
}

int snprintf_string(String* string, char* buf, size_t size) {
    return snprintf(buf, size, "%s", string->chars);
}

String* number_to_string(VirtualMachine* vm, Value* number) {
    char buffer[64];
    switch (number->type) {
        case VAL_I32: snprintf(buffer, sizeof(buffer), "%d", number->as.i32); break;
        case VAL_I64: snprintf(buffer, sizeof(buffer), "%" PRId64, (long long)number->as.i64); break;
        case VAL_F32: snprintf(buffer, sizeof(buffer), "%.6g", number->as.f32); break;
        case VAL_F64: snprintf(buffer, sizeof(buffer), "%.6g", number->as.f64); break;
        default: runtime_error(vm, "Cannot convert non-number to string"); return NULL;
    }
    return new_string(vm, buffer, (int)strlen(buffer));
}
