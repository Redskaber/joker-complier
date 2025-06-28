//
// Created by Kilig on 2024/11/21.
//

#include <stdlib.h>
#include <stdio.h>

#include "common.h"
#include "memory.h"
#include "allocator.h"
#include "vm.h"
#include "gc.h"


/*
* Reallocate memory for a pointer.
* If new_size is 0, the pointer is freed and NULL is returned.
* If realloc fails, an error message is printed and the program exits.
* Returns the new pointer.
*/
void* reallocate(VirtualMachine *vm, void* pointer, size_t old_size, size_t new_size) {

    vm->gc.bytes_allocated += (new_size - old_size);
    if (new_size > old_size) {
#if debug_stress_gc
        collect_garbage(vm);
#else
        if (vm->gc.bytes_allocated > vm->gc.next_gc) {
            collect_garbage(vm);
        }
#endif
    }

#if debug_enable_allocator
    if (pointer != NULL && new_size == 0) {
        free_memory(vm->allocator, pointer);
        return NULL;
	} else if (pointer == NULL && new_size > 0) {
        return allocate_memory(vm->allocator, new_size);
	} else if (pointer != NULL && new_size > 0) {
        void* new_pointer = reallocate_memory(vm->allocator, pointer, new_size);
        if (new_pointer == NULL) {
            fprintf(stderr, "Error: Failed to reallocate memory.\n");
            exit(EXIT_FAILURE);
        }
        return new_pointer;
    } else {
        return pointer;
    }
#else
    if (pointer != NULL && new_size == 0) {
        free(pointer);
        return NULL;
    }

    void* new_pointer = realloc(pointer, new_size);
    if (new_pointer == NULL) {
        fprintf(stderr, "Error: Failed to reallocate memory.\n");
        exit(EXIT_FAILURE);
    }
    return new_pointer;
#endif
}
