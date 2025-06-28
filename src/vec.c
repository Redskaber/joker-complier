//
// Created by Kilig on 2024/12/11.
//

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "object.h"
#include "error.h"
#include "memory.h"

#include "vec.h"

Vec* new_vec(VirtualMachine* vm) {
    Vec *vec = macro_allocate_object(vm, Vec, OBJ_VEC);
    vec->start = NULL;
    vec->finish = NULL;
    vec->end = NULL;
    vec->base.vtable = &vec_vtable;
    return vec;
}

Vec* new_vec_with_capacity(VirtualMachine* vm, size_t capacity) {
    Vec *vec = new_vec(vm);
    if (capacity > 0) {
        vec->start = macro_allocate(vm, Value, capacity);
        vec->finish = vec->start;
        vec->end = vec->start + capacity;
    }
    return vec;
}

static void vec_resize(Vec* vec, size_t min_capacity) {
    size_t old_capacity = vec_capacity(vec);
    size_t new_capacity = macro_grow_capacity(old_capacity);

    while (new_capacity < min_capacity) {
        new_capacity = macro_grow_capacity(new_capacity);
    }

    Value* old_start = vec->start;
    size_t old_size = vec_len(vec);

    Value* new_start = macro_allocate(vec->base.vm, Value, new_capacity);
    if (new_start == NULL) panic("{PANIC} [vec::resize] Failed to reallocate memory.");
    if (old_size > 0 && old_start != NULL) memcpy(new_start, old_start, old_size * sizeof(Value));
    if (old_start != NULL) macro_free(vec->base.vm, Value, old_start);

    vec->start = new_start;
    vec->finish = new_start + old_size;
    vec->end = new_start + new_capacity;
}


void free_vec(Vec* vec) {
    if (vec != NULL) {
        if (vec->start != NULL) {
            macro_free(vec->base.vm, Value, vec->start);
        }
        macro_free(vec->base.vm, Vec, vec);
    }
}

inline size_t vec_capacity(Vec* vec) {
    return vec->start == NULL ? 0: vec->end - vec->start;
}

inline size_t vec_len(Vec* vec) {
    return vec->start == NULL ? 0: vec->finish - vec->start;
}

void vec_push(Vec* vec, Value element) {
    if (vec_len(vec) == vec_capacity(vec)) {
        vec_resize(vec, vec_len(vec) + 1);
    }
    *vec->finish++ = element;
}

Value vec_pop(Vec* vec) {
    if (vec_is_empty(vec)) {
        panic("{PANIC} [vec::pop] Cannot pop element from empty vector.");
    }
    return *--vec->finish;
}

Value vec_first(Vec* vec) {
    if (vec_is_empty(vec)) {
        panic("{PANIC} [vec::first] Cannot get first element from empty vector.");
    }
    return vec->start[0];
}

Value vec_last(Vec* vec) {
    if (vec_is_empty(vec)) {
        panic("{PANIC} [vec::last] Cannot get last element from empty vector.");
    }
    Value rs = *--vec->finish;
    vec->finish++;
    return rs;
}

Value vec_get(Vec* vec, size_t index) {
    if (index >= vec_len(vec)) {
        panic("{PANIC} [vec::get] Index out of range.");
    }
    return vec->start[index];
}

void vec_set(Vec* vec, size_t index, Value element) {
    if (index >= vec_len(vec)) {
        panic("{PANIC} [vec::set] Index out of range.");
    }
    vec->start[index] = element;
}

void vec_insert(Vec* vec, size_t index, Value element) {
    size_t length = vec_len(vec);

    if (index > length) panic("{PANIC} [vec::insert] Index out of range.");
    if (length == vec_capacity(vec)) {
        vec_resize(vec, length + 1);
    }

    if (index == length) {
        *vec->finish++ = element;
    } else {
        for(size_t i = length; i > index; i--) {
            vec->start[i] = vec->start[i - 1];
        }
        vec->start[index] = element;
        vec->finish++;
    }
}

void vec_remove(Vec* vec, size_t index) {
    size_t length = vec_len(vec);
    if (index >= length) {
        panic("{PANIC} [vec::remove] Index out of range.");
    }
    for(size_t i = index; i < length - 1; i++) {
        vec->start[i] = vec->start[i + 1];
    }
    vec->finish--;
}

void vec_extend(Vec* vec, Vec* other) {
    if (other == NULL || vec_is_empty(other)) return;

    size_t len_v = vec_len(vec);
    size_t len_o = vec_len(other);
    size_t required_cap = len_v + len_o;

    if (required_cap > vec_capacity(vec)) {
        vec_resize(vec, required_cap);
    }

    memcpy(vec->finish, other->start, len_o * sizeof(Value));
    vec->finish += len_o;
}

void vec_clear(Vec* vec) {
    vec->finish = vec->start;
}

void vec_reverse(Vec* vec) {
    size_t length = vec_len(vec);
    for(size_t i = 0; i < length / 2; i++) {
        Value tmp = vec->start[i];
        vec->start[i] = vec->start[length - i - 1];
        vec->start[length - i - 1] = tmp;
    }
}

bool vec_is_empty(Vec* vec) {
    return vec->finish == vec->start;
}

void print_vec(Vec* vec) {
    size_t length = vec_len(vec);
    printf("Vec([ ");
    for(size_t i = 0; i < length; i++) {
        print_value(vec_get(vec, i));
        if (i + 1 < length) {
            printf(", ");
        }
    }
    printf(" ])\n");
}

/**
 * 将 Vec 结构格式化为字符串并写入缓冲区
 * @param vec   Vec 结构指针
 * @param buf   目标缓冲区
 * @param size  缓冲区最大容量
 * @return      成功写入的字符数（不含结尾 \0），失败返回负数
 */
int snprintf_vec(Vec* vec, char* buf, size_t size) {
    if (vec == NULL || buf == NULL || size == 0) {
        panic("{PANIC} Vec::snprintf_vec (vec == NULL || buf == NULL || size == 0)");
        return -1;
    }

    char* current = buf;        // 当前写入位置
    size_t remaining = size;    // 剩余缓冲区大小（含 \0 的空间）
    int written = 0;            // 单次写入的字符数
    int total = 0;              // 总写入字符数

    written = snprintf(current, remaining, "Vec{");
    if (written < 0 || (size_t)written >= remaining) {
        return (written < 0) ? written : (int)(size - 1);
    }
    current += written;
    remaining -= written;
    total += written;

    int count = (int)(vec->finish - vec->start);
    for (int i = 0; i < count; i++) {
        Value* element = vec->start + i;

        if (i > 0) {
            written = snprintf(current, remaining, ", ");
            if (written < 0 || (size_t)written >= remaining) {
                return (written < 0) ? written : total + (int)remaining - 1;
            }
            current += written;
            remaining -= written;
            total += written;
        }

        written = snprintf_value(*element, current, remaining);
        if (written < 0) {
            return written;  // 传递子函数错误
        } else if ((size_t)written >= remaining) {
            return total + (int)remaining - 1;  // 空间不足
        }
        current += written;
        remaining -= written;
        total += written;
    }

    written = snprintf(current, remaining, "}");
    if (written < 0 || (size_t)written >= remaining) {
        return (written < 0) ? written : total + (int)remaining - 1;
    }
    total += written;

    return total;
}

bool vec_equal(Vec* left, Vec* right) {
    if (left == right) return true;
    if(vec_len(left) != vec_len(right)) return false;
    for(size_t i = 0; i < vec_len(left); i++) {
        if(!values_equal(left->start[i], right->start[i])) {
            return false;
        }
    }
    return true;
}

