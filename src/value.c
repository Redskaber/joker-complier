//
// Created by Kilig on 2024/11/21.
//
#include <stdio.h>
#include <inttypes.h>  // 添加头文件以使用 PRId64

#include "memory.h"
#include "object.h"
#include "value.h"

void init_value_array(Values* array, VirtualMachine* vm) {
    array->vm = vm;
	array->count = 0;
	array->capacity = 0;
	array->values = NULL;
}

void free_value_array(Values* array) {
	macro_free_array(array->vm, Value, array->values, array->capacity);
    array->vm = NULL;
    array->count = 0;
    array->capacity = 0;
    array->values = NULL;
}

void write_value_array(Values* array, Value value) {
	if (array->capacity < array->count + 1) {
		int old_capacity = array->capacity;
		array->capacity = macro_grow_capacity(old_capacity);
		array->values = macro_grow_array(
                array->vm,
                Value,
                array->values,
                old_capacity,
                array->capacity
            );
	}

	array->values[array->count] = value;
	array->count++;
}

bool values_equal(Value left, Value right) {
#ifdef NAN_BOXING
    if (macro_is_number(left) && macro_is_number(right)) {
		return macro_as_number(left) == macro_as_number(right);
	}
    return left == right;
#else
    if (left.type != right.type) {
		return false;
	}
	switch (left.type) {
    case VAL_NULL:
    case VAL_NONE:	return true;
	case VAL_I32:	return macro_as_i32(left) == macro_as_i32(right);
	case VAL_I64:	return macro_as_i64(left) == macro_as_i64(right);
	case VAL_F32:	return macro_as_f32(left) == macro_as_f32(right);
	case VAL_F64:	return macro_as_f64(left) == macro_as_f64(right);
	case VAL_BOOL:	return macro_as_bool(left) == macro_as_bool(right);
	case VAL_OBJECT: return object_equal(macro_as_obj(left), macro_as_obj(right));
	default:		return false;
	}
#endif
}

void print_value(Value value) {
#ifdef NAN_BOXING
    if (macro_is_bool(value)) {
        printf(macro_as_bool(value) ? "true" : "false");
    } else if (macro_is_null(value)) {
        printf("null");
    } else if (macro_is_number(value)) {
        printf("%g", macro_as_number(value));
    } else if (macro_is_obj(value)) {
        print_object(macro_as_obj(value));
    }
#else
    switch (value.type) {
	case VAL_I32:   printf("%d", value.as.i32); break;
	case VAL_I64:   printf("%"   PRId64, value.as.i64); break;
	case VAL_F32:   printf("%f", value.as.f32); break;
	case VAL_F64:   printf("%f", value.as.f64); break;
	case VAL_BOOL:  printf("%s", value.as.boolean ? "true" : "false"); break;
    case VAL_NULL:  printf("null"); break;
    case VAL_NONE:  printf("None"); break;
	case VAL_OBJECT: print_object(macro_as_obj(value)); break;
	default:        warning("{WAINING} [value::print_value] Unknown value type"); break;
	}
#endif
}

void printf_value(Value value) {
	print_value(value);
	printf("\n");
}

// value.c
int snprintf_value(Value value, char* buf, size_t size) {
#ifdef NAN_BOXING
    if (macro_is_bool(value)) {
        snprintf(buf, size, "%s", macro_as_bool(value) ? "true" : "false");
    } else if (macro_is_null(value)) {
        snprintf(buf, size, "null");
    } else if (macro_is_obj(value)) {
        Object* obj = macro_as_obj(value);
        if (is_string(obj)) {
            String* str = (String*)obj;
            snprintf(buf, size, "%.*s", str->length, str->chars);
        } else {
            snprintf(buf, size, "<object>");
        }
    }
#else
    switch (value.type) {
        case VAL_I32:   return snprintf(buf, size, "%d", value.as.i32);
        case VAL_I64:   return snprintf(buf, size, "%" PRId64, value.as.i64);
        case VAL_F32:   return snprintf(buf, size, "%.2f", value.as.f32);
        case VAL_F64:   return snprintf(buf, size, "%.2f", value.as.f64);
        case VAL_BOOL:  return snprintf(buf, size, "%s", value.as.boolean ? "true" : "false");
        case VAL_NULL:  return snprintf(buf, size, "null");
        case VAL_NONE:  return snprintf(buf, size, "None");
        case VAL_OBJECT:return snprintf_object(macro_as_obj(value), buf, size);
        default:        return snprintf(buf, size, "<unknown>");
    }
#endif
}

