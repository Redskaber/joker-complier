//
// Created by Kilig on 2025/4/2.
//
#include <stdbool.h>
#include <string.h>

#include "class.h"
#include "hashmap.h"
#include "pair.h"
#include "vm.h"
#include "string_.h"
#include "type_register.h"


static void _type_register(VirtualMachine* self, String* name, Class* class);
static void define_class_native(VirtualMachine* self, Class* klass, const FnMapper(FnName, FnPtr) fn_mapper[2]);


void type_register(VirtualMachine *vm,
                   const char* type_name,
                   const ObjectVTable* object_vtable,
                   const FnMapper(FnName, FnPtr) methods[][2]
) {
    String* name = new_string(vm, type_name, (int)strlen(type_name));
    Class* klass = new_class(vm, name);
    if (klass == NULL) panic("[new_class_build_type] Failed to create class.");

    klass->base.vtable = object_vtable;

    int i = 0;
    const FnMapper(FnName, FnPtr)* fn_mapper = methods[i++];
    while(fn_mapper[0] != NULL && fn_mapper[1] != NULL) {
        define_class_native(vm, klass, fn_mapper);
        fn_mapper = methods[i++];
    }

    _type_register(vm, name, klass);
}

static void _type_register(VirtualMachine* self, String* name, Class* class) {
    push(self, macro_val_from_obj(name));
    push(self, macro_val_from_obj(class));
    hashmap_set(&self->types, macro_as_string(self->stack[0]), self->stack[1]);
    pop(self);
    pop(self);
}

static void define_class_native(VirtualMachine* self,
                                Class* klass,
                                const FnMapper(FnName, FnPtr) fn_mapper[2]
) {
    push(self, macro_val_from_obj(new_string(self, fn_mapper[0], strlen(fn_mapper[0]))));
    push(self, macro_val_from_obj(new_native(self, fn_mapper[1], true)));
    hashmap_set(&klass->methods, macro_as_string(self->stack[0]), self->stack[1]);
    pop(self);
    pop(self);
}

Class* type_find(VirtualMachine* vm, const char* type_name) {
    String* key = new_string(vm, type_name, (int)strlen(type_name));
    Value val = hashmap_get(&vm->types, key);
    return macro_is_class(val) ? macro_as_class(val) : NULL;
}

bool type_exists(VirtualMachine* vm, const char* type_name) {
    return type_find(vm, type_name) != NULL;
}


void type_dump_registered(VirtualMachine* vm) {
    printf("Registered Types:\n");
    for (int i = 0; i < vm->types.capacity; i++) {
        Entry* entry = &vm->types.entries[i];
        if (entry->key != NULL && values_equal(entry->value, macro_val_from_bool(true))) {
            String* type_name = (String*)entry->key;
            printf("  - %s\n", type_name->chars);
        }
    }
}
