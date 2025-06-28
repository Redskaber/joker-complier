//
// Created by Kilig on 2024/11/27.
//


#include "common.h"
#include "type.h"
#include "gc.h"
#include "vm.h"
#include "compiler.h"
#include "upvalue.h"
#include "memory.h"
#include "error.h"
#include "string_.h"
#include "class.h"
#include "struct_.h"
#include "bound_method.h"
#include "instance.h"
#include "pair.h"
#include "vec.h"
#include "enum.h"
#include "enum_instance.h"


#if debug_log_gc
#include <stdio.h>
#include "debug.h"
#endif
#if debug_trace_allocator
#include "allocator.h"
#endif


static void mark_roots(VirtualMachine *vm);
static void mark_value(VirtualMachine *vm, Value value);
static void mark_object(VirtualMachine *vm, Object* object);
static void mark_hashmap(VirtualMachine *vm, HashMap* hashmap);
static void mark_compiler_roots(VirtualMachine *vm, Compiler* compiler);

static void trace_references(VirtualMachine* vm);
static void blacken_object(VirtualMachine* vm, Object* object);
static void mark_vec(VirtualMachine* vm, Vec* vec);
static void mark_values(VirtualMachine* vm, Values* values);



Gc new_garbage_collector() {
    return (Gc) {
        .bytes_allocated = 0,
        .gray_capacity = 0,
        .gray_count = 0,
        .gray_stack = NULL,
        .next_gc = (1024 * 1024),
    };
}

void free_garbage_collector(Gc* self) {
    if (self != NULL) {
        free(self->gray_stack);
    }
}

static void print_unreached(Object* unreached) {
    printf("[gc::print_unreached] unreached object: ");
    print_object(unreached);
    printf("\n");
}

static void mark_roots(VirtualMachine *vm) {
    // stack
    for (Value* slot = vm->stack; slot < vm->stack_top; slot++) {
        mark_value(vm, *slot);
    }

    // call frames
    for (int i = 0; i < vm->frame_count; i++) {
        mark_object(vm, macro_into_object(vm->frames[i].closure));
    }

    // open upvalues
    for (Upvalue* upvalue = vm->open_upv_ptr;
        upvalue != NULL;
        upvalue = upvalue->next) {
        mark_object(vm, macro_into_object(upvalue->location));
    }

    // globals
    mark_hashmap(vm, &vm->globals);

    // compiler roots
    mark_compiler_roots(vm, vm->compiler);

    // init string
    mark_object(vm, macro_into_object(vm->init_string));
}

static void mark_value(VirtualMachine *vm, Value value) {
    if (macro_is_obj(value)) {
        mark_object(vm, macro_as_obj(value));
    }
}

static void mark_object(VirtualMachine *vm, Object* object) {
    if (object == NULL) return;
    if (object->is_marked) return;

#if debug_log_gc
    printf("[gc::mark_object] %p mark ", (void*)object);
    print_object(object);
    printf("\n");
#endif

    object->is_marked = true;

    // gray stack
    if (vm->gc.gray_capacity < vm->gc.gray_count + 1) {
        vm->gc.gray_capacity = macro_grow_capacity(vm->gc.gray_capacity);
#if debug_trace_allocator
        vm->gc.gray_stack = reallocate_memory(
            vm->allocator,
            vm->gc.gray_stack,
            vm->gc.gray_capacity * sizeof(Object*)
        );
#else
        void* new_stack = realloc(
                vm->gc.gray_stack,
                vm->gc.gray_capacity * sizeof(Object*)
        );
        if (new_stack == NULL) {
            panic(" {PANIC} [gc::mark_object] gc::gray_stack realloc memory fail.");
        }
        vm->gc.gray_stack = new_stack;
#endif
    }

    vm->gc.gray_stack[vm->gc.gray_count++] = object;
}

static void mark_hashmap(VirtualMachine *vm, HashMap* hashmap) {
    for (int i = 0; i < hashmap->capacity; i++) {
        Entry* entry = &hashmap->entries[i];
        if (!is_empty_entry(entry)) {
            mark_object(vm, macro_into_object(entry->key));
            mark_value(vm, entry->value);
        }
    }
}

static void mark_compiler_roots(VirtualMachine *vm, Compiler* compiler) {
    Compiler *curr = compiler;
    while (curr != NULL) {
        mark_object(vm, macro_into_object(curr->fn));
        curr = curr->enclosing;
    }
}

static void trace_references(VirtualMachine* vm) {
    while (vm->gc.gray_count > 0) {
        Object* object = vm->gc.gray_stack[--vm->gc.gray_count];
        blacken_object(vm, object);
    }
}

static void blacken_object(VirtualMachine* vm, Object* object) {
#if debug_log_gc
    printf("[gc::blacken_object] %p blacken ", (void*)object);
    print_object(object);
    printf("\n");
#endif

    switch (object->type) {
    case OBJ_TYPE: {
        Type* type = macro_as_type_from_obj(object);
        mark_object(vm, macro_into_object(type->name));
        mark_hashmap(vm, &type->methods);
        break;
    }
    case OBJ_PAIR: {
        Pair* pair = macro_as_pair_from_obj(object);
        mark_value(vm, pair->first);
        mark_value(vm, pair->second);
        break;
    }
    case OBJ_ENUM_INSTANCE: {
        EnumInstance* enum_instance = macro_as_enum_instance_from_obj(object);
        mark_object(vm, macro_into_object(enum_instance->name));
        mark_vec(vm, enum_instance->values);
        break;
    }
    case OBJ_ENUM: {
        Enum* enum_ = macro_as_enum_from_obj(object);
        mark_object(vm, macro_into_object(enum_->name));
        mark_hashmap(vm, &enum_->members);
        break;
    }
    case OBJ_VEC: {
        mark_vec(vm, macro_as_vec_from_obj(object));
        break;
    }
    case OBJ_STRUCT: {
        Struct* struct_ = macro_as_struct_from_obj(object);
        mark_hashmap(vm, &struct_->fields);
        mark_vec(vm, struct_->names);
        break;
    }
    case OBJ_BOUND_METHOD: {
        BoundMethod* bound = macro_as_bound_method_from_obj(object);
        mark_value(vm, bound->receiver);
        mark_object(vm, macro_into_object(bound->method));
        break;
    }
    case OBJ_INSTANCE: {
        Instance *instance = macro_as_instance_from_obj(object);
        mark_object(vm, macro_into_object(instance->klass));
        mark_hashmap(vm, &instance->fields);
        break;
    }
    case OBJ_CLASS: {
        Class* cls = macro_as_class_from_obj(object);
        mark_object(vm, macro_into_object(cls->name));
        mark_hashmap(vm, &cls->methods);
        break;
    }
    case OBJ_CLOSURE: {
        Closure* closure = macro_as_closure_from_obj(object);
        mark_object(vm, macro_into_object(closure->fn));
        for (int i = 0; i < closure->upvalue_count; i++) {
            mark_object(vm, macro_into_object(closure->upvalue_ptrs[i]));
        }
        break;
    }
    case OBJ_FN: {
        Fn* fn = macro_as_fn_from_obj(object);
        mark_object(vm, macro_into_object(fn->name));
        mark_values(vm, &fn->chunk.constants);
        break;
    }
    case OBJ_UPVALUE: mark_value(vm, macro_as_upvalue_from_obj(object)->closed); break;
    case OBJ_NATIVE:
    case OBJ_STRING:
        break;
    }
}

static void mark_vec(VirtualMachine* vm, Vec* vec) {
    for (size_t i = 0; i < vec_len(vec); i++) {
        mark_value(vm, vec->start[i]);
    }
}

static void mark_values(VirtualMachine* vm, Values* values) {
    for (int i = 0; i < values->count; i++) {
        mark_value(vm, values->values[i]);
    }
}

static void hashmap_remove_white(HashMap* hashmap) {
    for (int i = 0; i < hashmap->capacity; i++) {
        Entry* entry = &hashmap->entries[i];
        if (!is_empty_entry(entry) && !entry->key->base.is_marked) {
            hashmap_remove(hashmap, entry->key);
        }
    }
}

static void sweep(VirtualMachine* vm) {
    Object* prev = NULL;
    Object* object = vm->objects;

    while (object != NULL) {
        if (object->is_marked) {
            object->is_marked = false;  // unmark; next gc
            prev = object;
            object = object->next;
        } else {
            Object* unreached = object;
            object = object->next;
            if (prev != NULL) {
                prev->next = object;
            } else {
                vm->objects = object;
            }
            print_unreached(unreached);
            free_object(unreached);
        }
    }
}


void collect_garbage(VirtualMachine* vm) {
#if debug_log_gc
    printf("-- GC BEGIN\n");
    size_t before = vm->gc.bytes_allocated;
#endif
    // Used: mark-sweep
    mark_roots(vm);
    trace_references(vm);
    // weak ref and string pool
    hashmap_remove_white(&vm->strings);
    sweep(vm);

    // next gc
    vm->gc.next_gc = vm->gc.bytes_allocated * GC_HEAP_GROW_FACTOR;
#if debug_log_gc
    printf("-- GC END\n");
    size_t after = vm->gc.bytes_allocated;
    printf("[gc::collect_garbage] collected %I64d bytes (from %I64d to %I64d) next at %I64d\n",
           before - after,
           before,
           after,
           vm->gc.next_gc
       );
#endif
}

