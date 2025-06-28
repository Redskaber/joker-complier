//
// Created by Kilig on 2024/11/24.
//
/*
硬核模式：
 在不调用 realloc(), malloc(), 和 free()的前提下，实现reallocate()。
 你可以在解释器开始执行时调用一次malloc()，来分配一个大的内存块，
 你的reallocate()函数能够访问这个内存块。
 它可以从这个区域（你自己的私人堆内存）中分配内存块。
 你的工作就是定义如何做到这一点。
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "error.h"
#include "allocator.h"


static Part *new_part(PartSize size, const char *file, int line) {
    Part *part = malloc(sizeof(Part));

    size_t capacity = PART_COUNT[size][2] + sizeof(Block) * PART_COUNT[size][1];

    part->chunk = malloc(capacity);
    if (part->chunk == NULL) panic("[ {PANIC} memory::new_part] Out of memory.", MEMORY_ERROR_CODE);

    part->capacity = PART_COUNT[size][2];
    part->file = file;
    part->line = line;
    part->next = NULL;
    part->prev = NULL;
    part->size = size;
    part->timestamp = 0;
    part->total = PART_COUNT[size][1];
    part->used = 0;
    part->free_link = part->chunk;

    /* init part free linked */
    Block *block = part->chunk;
    for (size_t i = 0; i < part->total - 1; ++i) {
        block->part = part;
        block->is_used = false;
        block->next = (Block *) ((char *) block + sizeof(Block) + PART_COUNT[part->size][0]);
        block = block->next;
    }
    block->is_used = false;
    block->next = NULL;

    return part;
}

static void free_part(Part *part) {
    free(part->chunk);
    free(part);
}

static void print_part(Part *part) {
    printf("[memory::print_part] Part: %p\n", part);
    printf("[memory::print_part]   Capacity: %lu\n", (unsigned long)part->capacity);
    printf("[memory::print_part]   Chunk: %p\n", part->chunk);
    printf("[memory::print_part]   File: %s\n", part->file);
    printf("[memory::print_part]   Line: %d\n", part->line);
    printf("[memory::print_part]   Total: %lu\n", (unsigned long)part->total);
    printf("[memory::print_part]   Used: %lu\n", (unsigned long)part->used);
    printf("[memory::print_part]   Free: %lu\n", (unsigned long)(part->total - part->used));
    printf("[memory::print_part]   Free Link: %p\n", part->free_link);
}

////////////////////////////////////////////////////////////////////////////////

Allocator *new_allocator() {
    Allocator *allocator = malloc(sizeof(Allocator));
    allocator->head = NULL;
    allocator->total_capacity = 0;
    return allocator;
}

void free_allocator(Allocator *allocator) {
    Part *part = allocator->head;
    while (part != NULL) {
        Part *next = part->next;
        free_part(part);
        part = next;
    }
    free(allocator);
}

/* sorted part {smaller -> bigger}*/
static void add_part(Allocator *allocator, Part *part) {
    Part *prev = NULL;
    Part *current = allocator->head;
    while (current != NULL) {
        if (current == part) {
            panic("[ {PANIC} memory::add_part] Part already exists.", MEMORY_ERROR_CODE);
        }
        if (part->size < current->size) break;
        prev = current;
        current = current->next;
    }
    if (prev == NULL) {
        part->next = current;
        allocator->head = part;
        if (current != NULL) {
            current->prev = part;
        }
    } else {
        part->next = prev->next;
        prev->next = part;
        part->prev = prev;
        if (part->next != NULL) {
            part->next->prev = part;
        }
    }
    allocator->total_capacity += part->capacity;
}

static void remove_part(Allocator *allocator, Part *part) {
    Part *next = part->next;
    Part *prev = part->prev;

    if (next != NULL) next->prev = prev;
    if (prev != NULL) prev->next = next;
    else allocator->head = next;

    allocator->total_capacity -= part->capacity;
    free_part(part);
}

void* allocate_memory(Allocator *allocator, size_t size) {
#if debug_trace_allocator
    fprintf(stdout, "[memory::allocate_memory] Requested size: %lu\n", (unsigned long)size);
#endif
    if (size == 0) return NULL;

    PartSize part_size = MACRO_PART_SIZE_FROM_USER(size);
    if (part_size == PART_SIZE_OVERFLOW) {
        panic("[ {PANIC} memory::allocate_memory] Requested memory size is too big.", MEMORY_ERROR_CODE);
    }

    bool by_new = true;
    Part *part = allocator->head;
    while (true) {
        while (part != NULL) {
            if (part->size == part_size && part->free_link != NULL) {
                Block *block = part->free_link;
                part->free_link = block->next;
                part->used++;
                block->is_used = true;
                block->next = NULL;
                void* ra = (void*)(block) + sizeof(Block);
#if debug_trace_allocator
                fprintf(stdout, "[memory::allocate_memory] Allocated block at user address %p\n", ra);
#endif
                return ra;
            }
            part = part->next;
        }
        if (by_new) {
            part = new_part(part_size, __FILE__, __LINE__);
            add_part(allocator, part);
            by_new = false;
        } else {
            panic("[ {PANIC} memory::allocate_memory] Out of memory.", MEMORY_ERROR_CODE);
        }
    }
}


void *reallocate_memory(Allocator *allocator, void *memory, size_t new_size) {
    if (allocator == NULL) {
        panic("[ {PANIC} memory::reallocate_memory] Allocator is NULL.", MEMORY_ERROR_CODE);
    }

    if (new_size == 0) {
        free_memory(allocator, memory);
        return NULL;
    }

    if (memory == NULL) {
        return allocate_memory(allocator, new_size);
    } else {
        Block *block = (Block *) ((char *) memory - sizeof(Block));
        Part *part = block->part;
        size_t old_size = PART_COUNT[part->size][0];

        void *ra = allocate_memory(allocator, new_size);
        if (ra == NULL) {
            panic("[ {PANIC} memory::reallocate_memory] Out of memory.", MEMORY_ERROR_CODE);
        }

        size_t copy_size = (old_size < new_size) ? old_size : new_size;
        errno_t result = memcpy_s(ra, new_size, memory, copy_size);
        if (result != 0) {
            free_memory(allocator, ra);
            panic("[ {PANIC} memory::reallocate_memory] Failed to copy memory.", MEMORY_ERROR_CODE);
        }

        free_memory(allocator, memory);
        return ra;
    }
}


void free_memory(Allocator *allocator, void *memory) {
    if (memory == NULL) return;

    Block *block = (Block *) ((char *) memory - sizeof(Block));
    Part *part = block->part;

    if (part == NULL || !block->is_used || block->part != part) {
        panic("[ {PANIC} memory::free_memory] Invalid memory address.", MEMORY_ERROR_CODE);
    }

    part->used--;
    block->is_used = false;
    block->next = part->free_link;
    part->free_link = block;

#if debug_trace_allocator
    printf("[memory::free_memory] Freed at user address %p\n", memory);
#endif

    if(part->used == 0) {
        remove_part(allocator, part);
    }
}


void print_allocator(Allocator *allocator) {
    Part *part = allocator->head;
    while (part != NULL) {
        print_part(part);
        part = part->next;
    }
    fprintf(stdout, "[memory::print_allocator] Total Capacity: %lu\n", (unsigned long)allocator->total_capacity);
}
////////////////////////////////////////////////////////////////////////////////


void test_allocator() {
    Allocator *allocator = new_allocator();
    print_allocator(allocator);

    typedef struct Point {
        int x;
        int y;
    } Point;
    Point* point = (Point*)allocate_memory(allocator, sizeof(Point));
    point->x = 10;
    point->y = 20;
    printf("point: %d, %d\n", point->x, point->y);
    print_allocator(allocator);

    Point* point2 = (Point*)allocate_memory(allocator, sizeof(Point));
    point2->x = 30;
    point2->y = 40;
    printf("point2: %d, %d\n", point2->x, point2->y);
    print_allocator(allocator);

    typedef struct Foo {
        int a;
        int b;
        char c;
        char d;
    } Foo;

    Foo* foo = (Foo*)allocate_memory(allocator, sizeof(Foo));
    foo->a = 1;
    foo->b = 2;
    foo->c = 'a';
    foo->d = 'b';
    printf("foo: %d, %d, %c, %c\n", foo->a, foo->b, foo->c, foo->d);
    print_allocator(allocator);

    free_memory(allocator, point);
    free_memory(allocator, point2);

    free_memory(allocator, foo);

    print_allocator(allocator);

    /* reallocate_memory */
    typedef struct Foo2 {
        int *arr;
        int size;
        int capacity;
    } Foo2;

    Foo2* foo2 = (Foo2*)allocate_memory(allocator, sizeof(Foo2));
    foo2->size = 0;
    foo2->capacity = 4;
    foo2->arr = (int*)allocate_memory(allocator, sizeof(int) * foo2->capacity);
    for (int i = 0; i < 4; i++) {
        foo2->arr[i] = i;
        foo2->size++;
    }
    print_allocator(allocator);

    foo2 = (Foo2*)reallocate_memory(allocator, foo2, sizeof(Foo2) + sizeof(int) * 4);
    for (int i = 0; i < 4; i++) {
        foo2->arr[i] = i + 4;
    }
    print_allocator(allocator);
    free_memory(allocator, foo2->arr);

    free_allocator(allocator);
}
