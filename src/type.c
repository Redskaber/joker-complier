//
// Created by Kilig on 2024/12/22.
//

#include "common.h"
#include "string_.h"
#include "memory.h"
#include "object.h"
#include "type.h"


Type* new_type(VirtualMachine* vm, String* name) {
    Type *type = macro_allocate_object(vm, Type, OBJ_TYPE);
    type->name = name;
    init_hashmap(&type->methods, vm);
    return type;
}

void free_type(Type* type) {
    if (type != NULL) {
        free_hashmap(&type->methods);
        macro_free(type->base.vm, Type, type);
    }
}

void print_type(Type* type) {
    if (type != NULL) {
        printf("type %s", as_cstring(type->name));
    }
}

bool type_equal(Type* left, Type* right) {
    return left == right;
}

int snprintf_type(Type* type, char* buf, size_t size) {
    if (type != NULL) {
        return snprintf(buf, size, "type %s", as_cstring(type->name));
    }
    return 0;
}
