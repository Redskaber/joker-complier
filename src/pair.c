//
// Created by Kilig on 2024/12/13.
//

#include <stdio.h>
#include "memory.h"
#include "object.h"
#include "pair.h"


Pair *new_pair(VirtualMachine *vm, Value first, Value second) {
    Pair *pair = macro_allocate_object(vm, Pair, OBJ_PAIR);
    pair->first = first;
    pair->second = second;
    return pair;
}

void print_pair(Pair *self) {
    if (self != NULL) {
        printf("pair(");
        print_value(self->first);
        printf(", ");
        print_value(self->second);
        printf(")\n");
    }
}
void free_pair(Pair *self) {
    if (self != NULL) {
        macro_free(self->base.vm, Pair, self);
    }
}

bool pair_equal(Pair *left, Pair *right) {
    return values_equal(left->first, right->first) && values_equal(left->second, right->second);
}

/**
 * 将 Pair 结构格式化为字符串并写入缓冲区
 * @param self  Pair 结构指针
 * @param buf   目标缓冲区
 * @param size  缓冲区最大容量
 * @return      成功写入的字符数（不含结尾 \0），失败返回负数
 */
int snprintf_pair(Pair *self, char *buf, size_t size) {
    if (self == NULL || buf == NULL || size == 0) {
        panic("{PANIC} Pair::snprintf_pair self == NULL || buf == NULL || size == 0.");
        return -1;
    }

    char *current = buf;         // 当前写入位置
    size_t remaining = size;     // 剩余缓冲区大小（含 \0 的空间）
    int written = 0;             // 单次写入的字符数
    int total = 0;               // 总写入字符数

    written = snprintf(current, remaining, "Pair<");
    if (written < 0 || (size_t)written >= remaining) {
        return (written < 0) ? written : (int)(size - 1);  // 返回已写入或错误
    }
    current += written;
    remaining -= written;
    total += written;

    written = snprintf_value(self->first, current, remaining);
    if (written < 0) {
        return written;
    } else if ((size_t)written >= remaining) {
        return total + (int)remaining - 1;
    }
    current += written;
    remaining -= written;
    total += written;

    written = snprintf(current, remaining, ", ");
    if (written < 0 || (size_t)written >= remaining) {
        return (written < 0) ? written : total + (int)remaining - 1;
    }
    current += written;
    remaining -= written;
    total += written;

    written = snprintf_value(self->second, current, remaining);
    if (written < 0) {
        return written;
    } else if ((size_t)written >= remaining) {
        return total + (int)remaining - 1;
    }
    current += written;
    remaining -= written;
    total += written;

    written = snprintf(current, remaining, ">");
    if (written < 0 || (size_t)written >= remaining) {
        return (written < 0) ? written : total + (int)remaining - 1;
    }
    total += written;

    return total;
}
