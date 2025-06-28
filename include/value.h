//
// Created by Kilig on 2024/11/21.
//
#pragma once

#ifndef JOKER_ValueH
#define JOKER_ValueH
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "common.h"
#include "error.h"
#include "operator.h"

#ifdef NAN_BOXING  // improve performance 35% faster
#include <string.h>
#define SIGN_BIT    ((uint64_t)0x8000000000000000)
#define QNAN        ((uint64_t)0x7ffc000000000000)


#define TAG_NULL    1  // 01
#define TAG_FALSE   2  // 10
#define TAG_TRUE    3  // 11


// #define IS_BOOL(v) ((v) == TRUE_VAL || (v) == FALSE_VAL)
#define macro_is_bool(value)    (((value) | 1) == macro_true_val)
#define macro_is_null(value)    ((value) == macro_null_val)
#define macro_is_obj(value) \
    (((value) & (QNAN | SIGN_BIT)) == (QNAN | SIGN_BIT))

#define macro_is_number(value)  (((value) & QNAN) != QNAN)
#define macro_as_bool(value)    ((value) == macro_true_val)
#define macro_as_number(value)  Valueto_num(value)
#define macro_as_obj(value) \
    ((Object*)(uintptr_t)((value) & ~(SIGN_BIT | QNAN)))


#define macro_false_val         ((Value)(uint64_t)(QNAN | TAG_FALSE))
#define macro_true_val          ((Value)(uint64_t)(QNAN | TAG_TRUE))
#define macro_null_val		    ((Value)(uint64_t)(QNAN | TAG_NULL))
#define macro_bool_val(b)       ((b) ? macro_true_val : macro_false_val)
#define macro_number_val(num)   num_to_value(num)
#define macro_obj_val(obj) \
    (Value)(SIGN_BIT | QNAN | (uint64_t)(uintptr_t)(obj))


static inline double value_to_num(Value value) {
    double num;
    memcpy(&num, &value, sizeof(num));
    return num;
}

static inline Value num_to_value(double num) {
    Value value;
    memcpy(&value, &num, sizeof(num));
    return value;
}

#else
/* 值类型 */
typedef enum {
	VAL_I32,
	VAL_I64,
	VAL_F32,
	VAL_F64,
	VAL_BOOL,
	VAL_OBJECT,
    VAL_NONE,
    VAL_NULL,
} ValueType;

/* 值: 存储在栈中的值
*
* TODO: Value Hashes
*/
typedef struct Value {
	ValueType type;
	union as {
		int32_t		i32;
		int64_t		i64;
		float		f32;
		double		f64;
		bool	boolean;
		Object* object;
	} as;
} Value;


#define VALUE_COUNT 8
#define macro_matches(value, t)			((value).type == (t))
#define macro_matches_ptr(value_ptr, t) ((value_ptr) && (value_ptr)->type == (t))

#define macro_val_from_i32(i32_)	((Value){ VAL_I32,	{ .i32 = (i32_) } })
#define macro_val_from_i64(i64_)	((Value){ VAL_I64,	{ .i64 = (i64_) } })
#define macro_val_from_f32(f32_)	((Value){ VAL_F32,	{ .f32 = (f32_) } })
#define macro_val_from_f64(f64_)	((Value){ VAL_F64,	{ .f64 = (f64_) } })
#define macro_val_from_bool(bool_)	((Value){ VAL_BOOL, { .boolean = (bool_) } })
#define macro_val_from_obj(obj_)	((Value){ VAL_OBJECT, { .object = (Object*)(obj_) } })  // value need ownership of object
#define macro_val_null				((Value){ VAL_NULL, { .i32 = 0 } })
#define macro_val_none				((Value){ VAL_NONE, { .i32 = 0 } })

#define macro_is_i32(value)		((value).type == VAL_I32)
#define macro_is_i64(value)		((value).type == VAL_I64)
#define macro_is_f32(value)		((value).type == VAL_F32)
#define macro_is_f64(value)		((value).type == VAL_F64)
#define macro_is_bool(value)	((value).type == VAL_BOOL)
#define macro_is_obj(value)		((value).type == VAL_OBJECT)
#define macro_is_none(value)	((value).type == VAL_NONE)
#define macro_is_null(value)	((value).type == VAL_NULL)      // hashmap empty value slot default
#define macro_is_number(value)		  \
    (macro_matches(value, VAL_I32) || \
	 macro_matches(value, VAL_I64) || \
	 macro_matches(value, VAL_F32) || \
     macro_matches(value, VAL_F64)	  \
	)
#define macro_is_obj_ptr(value_ptr)		((value_ptr) && (value_ptr)->type == VAL_OBJECT)

#define macro_set_i32(value_ptr, value) \
	do {								\
		(value_ptr)->type = VAL_I32;	\
		(value_ptr)->as.i32 = (value);	\
	} while (0)

#define macro_set_i64(value_ptr, value) \
	do {								\
		(value_ptr)->type = VAL_I64;	\
		(value_ptr)->as.i64 = (value);	\
	} while (0)

#define macro_set_f32(value_ptr, value) \
	do {								\
		(value_ptr)->type = VAL_F32;	\
		(value_ptr)->as.f32 = (value);	\
	} while (0)

#define macro_set_f64(value_ptr, value) \
	do {								\
		(value_ptr)->type = VAL_F64;	\
		(value_ptr)->as.f64 = (value);	\
	} while (0)

#define macro_set_bool(value_ptr, value) \
	do {								\
		(value_ptr)->type = VAL_BOOL;	\
		(value_ptr)->as.boolean = (value);	\
	} while (0)

#define macro_set_obj(value_ptr, value) \
	do {								\
		(value_ptr)->type = VAL_OBJECT;	\
		(value_ptr)->as.object = (value);	\
	} while (0)

#define macro_set_none(value_ptr)       \
	do {								\
		(value_ptr)->type = VAL_NONE;	\
	} while (0)



#define macro_as_i32(value)		((value).as.i32)
#define macro_as_i64(value)		((value).as.i64)
#define macro_as_f32(value)		((value).as.f32)
#define macro_as_f64(value)		((value).as.f64)
#define macro_as_bool(value)	((value).as.boolean)
#define macro_as_obj(value)		((value).as.object)
#define macro_as_obj_ptr(value_ptr)		((value_ptr)->as.object)
#define macro_as_i32_ptr(value_ptr)		((value_ptr)->as.i32)
#define macro_as_i64_ptr(value_ptr)		((value_ptr)->as.i64)
#define macro_as_f32_ptr(value_ptr)		((value_ptr)->as.f32)
#define macro_as_f64_ptr(value_ptr)		((value_ptr)->as.f64)
#define macro_as_bool_ptr(value_ptr)		((value_ptr)->as.boolean)

#define macro_type_name(value)					\
	(											\
		(value).type == VAL_I32 ? "i32" :		\
		(value).type == VAL_I64 ? "i64" :		\
		(value).type == VAL_F32 ? "f32" :		\
		(value).type == VAL_F64 ? "f64" :		\
		(value).type == VAL_BOOL? "bool" :		\
		(value).type == VAL_NULL? "null" :		\
		(value).type == VAL_NONE? "None" :		\
		(value).type == VAL_OBJECT? "Object" :	\
		"Unknown"								\
	)
#define macro_type_name_ptr(value_ptr)	macro_type_name(*value_ptr)
#define macro_check_nullptr(value_ptr)					\
	do {												\
		if (!(value_ptr)) {                             \
            panic("[ {PANIC} Value::macro_check_nullptr] Nullptr value pointer."); \
		}												\
	} while (0)
#endif


#define macro_to_f32(value_ptr) do {    \
    value_ptr->type = VAL_F32;          \
    value_ptr->as.f32 = (float)value_to_i32(*value_ptr); \
} while(0)

#define macro_to_f64(value_ptr) do {    \
    value_ptr->type = VAL_F64;          \
    value_ptr->as.f64 = (double)value_to_i32(*value_ptr); \
} while(0)

#define macro_to_i32(value_ptr) do {    \
    value_ptr->type = VAL_I32;          \
    value_ptr->as.i32 = value_to_i32(*value_ptr); \
} while(0)

#define macro_to_i64(value_ptr) do {    \
    value_ptr->type = VAL_I64;          \
    value_ptr->as.i64 = value_to_i64(*value_ptr); \
} while(0)


#define MACRO_OP_NOT_SUPPORTED(type, op) \
    (runtime_error(vm, "Operator '%s' not supported for %s", macro_ops_to_string(op), type), \
     interpret_runtime_error)

#define MACRO_CHECK_I32_OVERFLOW(a, op, b) \
    do { \
        int64_t result = (int64_t)a op (int64_t)b; \
        if (result < INT32_MIN || result > INT32_MAX) { \
            runtime_error(vm, "i32 overflow in operation"); \
            return interpret_runtime_error; \
        } \
    } while(0)



// 值数组类型
typedef struct Values {
    VirtualMachine *vm;
	int32_t count;
	int32_t capacity;
	Value* values;
} Values;

void init_value_array(Values* array, VirtualMachine* vm);
void free_value_array(Values* array);
void write_value_array(Values* array, Value value);
bool values_equal(Value left, Value right);

void print_value(Value value);
void printf_value(Value value);
int snprintf_value(Value value, char* buf, size_t size);


static inline int32_t __attribute__((unused)) value_to_i32(Value val) {
    switch(val.type) {
        case VAL_I32: return val.as.i32;
        case VAL_I64: return (int32_t)val.as.i64;
        case VAL_F32: return (int32_t)val.as.f32;
        case VAL_F64: return (int32_t)val.as.f64;
        default: return 0;
    }
}
static inline int64_t __attribute__((unused)) value_to_i64(Value val) {
    switch(val.type) {
        case VAL_I32: return (int64_t)val.as.i32;
        case VAL_I64: return val.as.i64;
        case VAL_F32: return (int64_t)val.as.f32;
        case VAL_F64: return (int64_t)val.as.f64;
        default: return 0;
    }
}
static inline float __attribute__((unused)) value_to_f32(Value val) {
    switch(val.type) {
        case VAL_I32: return (float)val.as.i32;
        case VAL_I64: return (float)val.as.i64;
        case VAL_F32: return val.as.f32;
        case VAL_F64: return (float)val.as.f64;
        default: return 0.0f;
    }
}
static inline double __attribute__((unused)) value_to_f64(Value val) {
    switch(val.type) {
        case VAL_I32: return (double)val.as.i32;
        case VAL_I64: return (double)val.as.i64;
        case VAL_F32: return (double)val.as.f32;
        case VAL_F64: return val.as.f64;
        default: return 0.0;
    }
}

static inline void __attribute__((unused)) numeric_type_promotion(Value* a, Value* b) {
    if (a->type == VAL_F64 || b->type == VAL_F64) {
        macro_to_f64(a);
        macro_to_f64(b);
    } else if (a->type == VAL_F32 || b->type == VAL_F32) {
        macro_to_f32(a);
        macro_to_f32(b);
    } else if (a->type == VAL_I64 || b->type == VAL_I64) {
        macro_to_i64(a);
        macro_to_i64(b);
    }
}

#endif //JOKER_ValueH
