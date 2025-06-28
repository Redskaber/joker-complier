//
// Created by Kilig on 2025/4/2.
//

#include "class.h"
#include "vm.h"
#include "value.h"
#include "vec.h"
#include "instance.h"
#include "string_.h"
#include "../include/vec.h"


const FnMapper(FnName, FnPtr) vec_export_methods[][2] = {
        {"get",     native_vec_get},
        {"set",     native_vec_set},
        {"push",    native_vec_push},
        {"pop",     native_vec_pop},
        {"insert",  native_vec_insert},
        {"remove",  native_vec_remove},
        {"extend",  native_vec_extend},
        {"clear",   native_vec_clear},
        {"reverse", native_vec_reverse},
        {"len",     native_vec_length},
        {"first",   native_vec_first},
        {"last",    native_vec_last},
        {NULL,      NULL}
};


Value native_vec_new(VirtualMachine* vm, int arg_count, Value* args) {
    (void)args;

    if (arg_count != 0) {
        runtime_error(vm, "Expected 0 arguments for 'new'.");
        return macro_val_null;
    }

    Class* klass = macro_as_class(hashmap_get(&vm->types, new_string(vm, "Vec", 3)));
    Instance* instance = new_instance(vm, klass);
    Vec* data = new_vec(vm);
    hashmap_set(&instance->fields, new_string(vm, "_data", 5), macro_val_from_obj(data));
    return macro_val_from_obj(instance);
}

Value native_vec_get(VirtualMachine* vm, int arg_count, Value* args) {
    if (arg_count != 2 || !macro_is_i32(args[1])) {
        runtime_error(vm, "Expected 1 integer argument for 'get'.");
        return macro_val_null;
    }

    Instance* instance = macro_as_instance(args[0]);
    Value data_val = hashmap_get(&instance->fields, new_string(vm, "_data", 5));
    if (!macro_is_vec(data_val)) {
        runtime_error(vm, "Vector data corrupted.");
        return macro_val_null;
    }

    Vec* vec = macro_as_vec(data_val);
    int32_t index = macro_as_i32(args[1]);
    if (index < 0 || index >= (int32_t)vec_len(vec)) {
        runtime_error(vm, "Index %d out of bounds (size=%d).", index, vec_len(vec));
        return macro_val_null;
    }

    return vec_get(vec, index);
}

Value native_vec_set(VirtualMachine* vm, int arg_count, Value* args) {
    if (arg_count != 3 || !macro_is_i32(args[1])) {
        runtime_error(vm, "Expected 2 integer argument for 'set'.");
        return macro_val_null;
    }

    Instance* instance = macro_as_instance(args[0]);
    Value data_val = hashmap_get(&instance->fields, new_string(vm, "_data", 5));
    if (!macro_is_vec(data_val)) {
        runtime_error(vm, "Vector data corrupted.");
        return macro_val_null;
    }

    Vec* vec = macro_as_vec(data_val);
    int32_t index = macro_as_i32(args[1]);
    if (index < 0 || index >= (int32_t)vec_len(vec)) {
        runtime_error(vm, "Index %d out of bounds (size=%d).", index, vec_len(vec));
        return macro_val_null;
    }

    vec_set(vec, index, args[2]);
    return macro_val_null;
}

Value native_vec_push(VirtualMachine* vm, int arg_count, Value* args) {
    if (arg_count != 2) {
        runtime_error(vm, "Expected 1 argument for 'push'.");
        return macro_val_null;
    }

    Instance* instance = macro_as_instance(args[0]);
    Value data_val = hashmap_get(&instance->fields, new_string(vm, "_data", 5));
    if (!macro_is_vec(data_val)) {
        runtime_error(vm, "Vector data corrupted.");
        return macro_val_null;
    }

    vec_push(macro_as_vec(data_val), args[1]);
    return macro_val_null;
}

Value native_vec_pop(VirtualMachine* vm, int arg_count, Value* args) {
    if (arg_count != 1) {
        runtime_error(vm, "Expected 0 arguments for 'pop'.");
        return macro_val_null;
    }

    Instance* instance = macro_as_instance(args[0]);
    Value data_val = hashmap_get(&instance->fields, new_string(vm, "_data", 5));
    if (!macro_is_vec(data_val)){
        runtime_error(vm, "Vector data corrupted.");
        return macro_val_null;
    }

    return vec_pop(macro_as_vec(data_val));
}

Value native_vec_insert(VirtualMachine* vm, int arg_count, Value* args) {
    if (arg_count != 3 || !macro_is_i32(args[1])) {
        runtime_error(vm, "Expected 2 integer argument for 'insert'.");
        return macro_val_null;
    }

    Instance* instance = macro_as_instance(args[0]);
    Value data_val = hashmap_get(&instance->fields, new_string(vm, "_data", 5));
    if (!macro_is_vec(data_val)) {
        runtime_error(vm, "Vector data corrupted.");
        return macro_val_null;
    }

    Vec* vec = macro_as_vec(data_val);
    int32_t index = macro_as_i32(args[1]);
    if (index < 0 || index > (int32_t)vec_len(vec)) {
        runtime_error(vm, "Index %d out of bounds (size=%d).", index, vec_len(vec));
        return macro_val_null;
    }

    vec_insert(vec, index, args[2]);
    return macro_val_null;
}

Value native_vec_remove(VirtualMachine* vm, int arg_count, Value* args) {
    if (arg_count != 2 || !macro_is_i32(args[1])) {
        runtime_error(vm, "Expected 1 integer argument for 'remove'.");
        return macro_val_null;
    }

    Instance* instance = macro_as_instance(args[0]);
    Value data_val = hashmap_get(&instance->fields, new_string(vm, "_data", 5));
    if (!macro_is_vec(data_val)) {
        runtime_error(vm, "Vector data corrupted.");
        return macro_val_null;
    }

    int index = macro_as_i32(args[1]);
    if (index < 0 || index >= (int32_t)vec_len(macro_as_vec(data_val))) {
        runtime_error(vm, "Index %d out of bounds (size=%d).", index, vec_len(macro_as_vec(data_val)));
        return macro_val_null;
    }

    vec_remove(macro_as_vec(data_val), index);
    return macro_val_null;
}

Value native_vec_extend(VirtualMachine* vm, int arg_count, Value* args) {
    if (arg_count != 2) {
        runtime_error(vm, "Expected 1 argument for 'extend'.");
        return macro_val_null;
    }

    Instance* to_vec = macro_as_instance(args[0]);
    Value to_data = hashmap_get(&to_vec->fields, new_string(vm, "_data", 5));
    if (!macro_is_vec(to_data)){
        runtime_error(vm, "Vector data corrupted.");
        return macro_val_null;
    }

    Instance* from_vec = macro_as_instance(args[1]);
    Value from_data = hashmap_get(&from_vec->fields, new_string(vm, "_data", 5));
    if (!macro_is_vec(from_data)) {
        runtime_error(vm, "Vector data corrupted.");
        return macro_val_null;
    }

    vec_extend(macro_as_vec(to_data), macro_as_vec(from_data));
    return macro_val_null;
}

Value native_vec_clear(VirtualMachine* vm, int arg_count, Value* args) {
    if (arg_count != 1) {
        runtime_error(vm, "Expected 0 arguments for 'clear'.");
        return macro_val_null;
    }

    Instance* instance = macro_as_instance(args[0]);
    Value data_val = hashmap_get(&instance->fields, new_string(vm, "_data", 5));
    if (!macro_is_vec(data_val)){
        runtime_error(vm, "Vector data corrupted.");
        return macro_val_null;
    }
    vec_clear(macro_as_vec(data_val));
    return macro_val_null;
}

Value native_vec_reverse(VirtualMachine* vm, int arg_count, Value* args) {
    if (arg_count != 1) {
        runtime_error(vm, "Expected 0 arguments for 'reverse'.");
        return macro_val_null;
    }

    Instance* instance = macro_as_instance(args[0]);
    Value data_val = hashmap_get(&instance->fields, new_string(vm, "_data", 5));
    if (!macro_is_vec(data_val)) {
        runtime_error(vm, "Vector data corrupted.");
        return macro_val_null;
    }

    vec_reverse(macro_as_vec(data_val));
    return macro_val_null;
}

Value native_vec_length(VirtualMachine* vm, int arg_count, Value* args) {
    if (arg_count != 1) {
        runtime_error(vm, "Expected 0 arguments for 'length'.");
        return macro_val_null;
    }

    Instance* instance = macro_as_instance(args[0]);
    Value data_val = hashmap_get(&instance->fields, new_string(vm, "_data", 5));
    if (!macro_is_vec(data_val)) {
        runtime_error(vm, "Vector data corrupted.");
        return macro_val_null;
    }

    Vec* vec = macro_as_vec(data_val);
    return macro_val_from_i32(vec_len(vec));
}

Value native_vec_first(VirtualMachine* vm, int arg_count, Value* args) {
    if (arg_count != 1) {
        runtime_error(vm, "Expected 0 arguments for 'first'.");
        return macro_val_null;
    }

    Instance* instance = macro_as_instance(args[0]);
    Value data_val = hashmap_get(&instance->fields, new_string(vm, "_data", 5));
    if (!macro_is_vec(data_val)) {
        runtime_error(vm, "Vector data corrupted.");
        return macro_val_null;
    }
    return vec_first(macro_as_vec(data_val));
}

Value native_vec_last(VirtualMachine* vm, int arg_count, Value* args) {
    if (arg_count != 1) {
        runtime_error(vm, "Expected 0 arguments for 'last'.");
        return macro_val_null;
    }

    Instance* instance = macro_as_instance(args[0]);
    Value data_val = hashmap_get(&instance->fields, new_string(vm, "_data", 5));
    if (!macro_is_vec(data_val)){
        runtime_error(vm, "Vector data corrupted.");
        return macro_val_null;
    }
    return vec_last(macro_as_vec(data_val));
}
