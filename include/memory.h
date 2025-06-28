//
// Created by Kilig on 2024/11/21.
//
#pragma once

#ifndef JOKER_MEMORY_H
#define JOKER_MEMORY_H
#include "common.h"

// macro for allocate array
#define macro_allocate(vm, type, capacity) \
	(type*)reallocate(vm, NULL, 0, sizeof(type) * (capacity))

// macro for free pointer
#define macro_free(vm, type, pointer) \
    reallocate(vm, pointer, sizeof(type), 0)

// macro for grow capacity
#define macro_grow_capacity(capacity) ((capacity) < 8 ? 8 : (capacity) * 2)
// macro for grow array
#define macro_grow_array(vm, type, pointer, old_capacity, new_capacity) \
	(type*)reallocate(                                              \
        vm,                                                         \
		pointer,													\
		sizeof(type) *(old_capacity),								\
        sizeof(type) *(new_capacity)								\
	)
// macro for free array
#define macro_free_array(vm, type, pointer, old_capacity) \
    reallocate(											\
        vm,                                             \
		pointer,										\
		sizeof(type) * (old_capacity),					\
		0												\
	)

/*
* reallocate memory
*
	oldSize	 new_size	 operator
		0	   !0			Allocate new block.			分配新块
	   !0		0			Free allocation.			释放已分配内存
	   !0		< oldSize	Shrink existing allocation. 收缩已分配内存
	   !0		> oldSize	Grow existing allocation.	增加已分配内存
*/
void* reallocate(VirtualMachine *vm, void* pointer, size_t old_size, size_t new_size);

#endif //JOKER_MEMORY_H
