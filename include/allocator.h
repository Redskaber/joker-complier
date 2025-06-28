//
// Created by Kilig on 2024/11/24.
//
#pragma once

#ifndef JOKER_ALLOCATOR_H
#define JOKER_ALLOCATOR_H
#include "common.h"

#define MEMORY_ERROR_CODE (-1)


/// \brief Allocator
/// Buddy systems
typedef enum {
    PART_SIZE_8,                // 8B * 128     = 1KB
    PART_SIZE_16,               // 16B * 256    = 4KB
    PART_SIZE_32,               // 32B * 512    = 16KB
    PART_SIZE_64,               // 64B * 2048   = 128KB
    PART_SIZE_128,              // 128B * 4096  = 512KB
    PART_SIZE_256,              // 256B * 4096  = 1MB
    PART_SIZE_512,              // 512B * 1024
    PART_SIZE_1024,             // 1KB * 512
    PART_SIZE_2048,             // 2KB * 256
    PART_SIZE_4096,             // 4KB * 128
    PART_SIZE_8192,             // 8KB * 64
    PART_SIZE_16384,            // 16KB * 32
    PART_SIZE_32768,            // 32KB * 16
    PART_SIZE_65536,            // 64KB * 8
    PART_SIZE_OVERFLOW          //
} PartSize;

#define MACRO_PART_SIZE_STRING(SIZE) \
    (SIZE == PART_SIZE_8) ? "PART_SIZE_8" : \
    (SIZE == PART_SIZE_16) ? "PART_SIZE_16" : \
    (SIZE == PART_SIZE_32) ? "PART_SIZE_32" : \
    (SIZE == PART_SIZE_64) ? "PART_SIZE_64" : \
    (SIZE == PART_SIZE_128) ? "PART_SIZE_128" : \
    (SIZE == PART_SIZE_256) ? "PART_SIZE_256" : \
    (SIZE == PART_SIZE_512) ? "PART_SIZE_512" : \
    (SIZE == PART_SIZE_1024) ? "PART_SIZE_1024" : \
    (SIZE == PART_SIZE_2048) ? "PART_SIZE_2048" : \
    (SIZE == PART_SIZE_4096) ? "PART_SIZE_4096" : \
    (SIZE == PART_SIZE_8192) ? "PART_SIZE_8192" : \
    (SIZE == PART_SIZE_16384) ? "PART_SIZE_16384" : \
    (SIZE == PART_SIZE_32768) ? "PART_SIZE_32768" : \
    (SIZE == PART_SIZE_65536) ? "PART_SIZE_65536" : \
    "Overflow"

/* TODO: handle overflow user size <= Min(PartSize) */
#define MACRO_PART_SIZE_FROM_USER(size) \
    (size <= PART_COUNT[PART_SIZE_8][0]) ? PART_SIZE_8 : \
    (size <= PART_COUNT[PART_SIZE_16][0]) ? PART_SIZE_16 : \
    (size <= PART_COUNT[PART_SIZE_32][0]) ? PART_SIZE_32 : \
    (size <= PART_COUNT[PART_SIZE_64][0]) ? PART_SIZE_64 : \
    (size <= PART_COUNT[PART_SIZE_128][0]) ? PART_SIZE_128 :\
    (size <= PART_COUNT[PART_SIZE_256][0]) ? PART_SIZE_256 :\
    (size <= PART_COUNT[PART_SIZE_512][0]) ? PART_SIZE_512 :\
    (size <= PART_COUNT[PART_SIZE_1024][0]) ? PART_SIZE_1024 :\
    (size <= PART_COUNT[PART_SIZE_2048][0]) ? PART_SIZE_2048 :\
    (size <= PART_COUNT[PART_SIZE_4096][0]) ? PART_SIZE_4096 :\
    (size <= PART_COUNT[PART_SIZE_8192][0]) ? PART_SIZE_8192 :\
    (size <= PART_COUNT[PART_SIZE_16384][0]) ? PART_SIZE_16384 :\
    (size <= PART_COUNT[PART_SIZE_32768][0]) ? PART_SIZE_32768 :\
    (size <= PART_COUNT[PART_SIZE_65536][0]) ? PART_SIZE_65536 :\
    PART_SIZE_OVERFLOW

static const size_t PART_COUNT[][4] = {
        [PART_SIZE_8]       = {8,   128, 1024, 3},
        [PART_SIZE_16]      = {16,  256, 4096, 4},
        [PART_SIZE_32]      = {32,  512, 16384, 5},
        [PART_SIZE_64]      = {64,  1024,65536, 6},
        [PART_SIZE_128]     = {128, 512, 65536, 7},
        [PART_SIZE_256]     = {256, 256, 65536, 8},
        [PART_SIZE_512]     = {512, 128, 65536, 9},
        [PART_SIZE_1024]    = {1024, 64, 65536, 10},
        [PART_SIZE_2048]    = {2048, 32, 65536, 11},
        [PART_SIZE_4096]    = {4096, 16, 65536, 12},
        [PART_SIZE_8192]    = {8192, 8,  65536, 13},
        [PART_SIZE_16384]   = {16384, 4, 65536, 14},
        [PART_SIZE_32768]   = {32768, 2, 65536, 15},
        [PART_SIZE_65536]   = {65535, 1, 65536, 16},
};

typedef struct Part {
    void *chunk;                // 内存块指针
    size_t capacity;            // 内存块容量
    size_t total;               // 内存块总数量
    size_t used;                // 内存块中已分配内存块数量

    PartSize size;              // 内存块大小
    unsigned long timestamp;    // 内存分配时间戳
    const char* file;           // 分配内存的文件名
    int line;                   // 分配内存的行号

    struct Block* free_link;    // 内存块空闲链表
    struct Part* next;          // 下一个内存块
    struct Part* prev;          // 上一个内存块
} Part;

// typedef uint32_t Block;
// [high is_used[1bit], unused[3bit], [power[4bit]], index[12bit], next_offset[12bit] low];

typedef struct Block {
    Part* part;                 // 所属内存块
    bool is_used;               // 是否被使用
    struct Block* next;         // 下一个内存块
} Block;

typedef struct Allocator {
    Part* head;
    size_t total_capacity;
} Allocator;


Allocator *new_allocator();
void free_allocator(Allocator *allocator);
void* allocate_memory(Allocator *allocator, size_t size);
void *reallocate_memory(Allocator *allocator, void *memory, size_t new_size);
void free_memory(Allocator *allocator, void *memory);
void print_allocator(Allocator *allocator);

void test_meta();
void test_allocator();
#endif //JOKER_ALLOCATOR_H
