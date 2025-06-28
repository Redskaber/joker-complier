//
// Created by Kilig on 2024/11/27.
//
#pragma once

#ifndef JOKER_GC_H
#define JOKER_GC_H
#include "common.h"
#define GC_HEAP_GROW_FACTOR 2

typedef struct GarbageCollector {
    int gray_count;
    int gray_capacity;
    Object** gray_stack;
    size_t bytes_allocated;
    size_t next_gc;
} Gc;

Gc new_garbage_collector();
void free_garbage_collector(Gc* self);

void collect_garbage(VirtualMachine* vm);

#endif //JOKER_GC_H
