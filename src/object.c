//
// Created by Kilig on 2024/11/21.
//

/*
* object.c
*  Object(struct) <-----> Operation(trait[interface])
*/

#include <string_.h>

#include "error.h"
#include "memory.h"
#include "object.h"
#include "type.h"
#include "native.h"
#include "fn.h"
#include "closure.h"
#include "struct_.h"
#include "class.h"
#include "instance.h"
#include "bound_method.h"
#include "upvalue.h"
#include "pair.h"
#include "vec.h"
#include "enum.h"
#include "enum_instance.h"
#include "vm.h"


/*
* Allocate a new object of the given size and type.
*
* -------Object Layout------
* |  |-----Sub Object----| |
* |  |                   | |
* |  |                   | |
* |  |-------------------| |
* --------------------------
*
* Object Size = SubObject Size
* The object is just the shell of the concrete object,
* and the specific size of the object is determined by the specific type.
*
* The sub object is the actual object data, and its size is determined by the specific type.
*/
Object* allocate_object(VirtualMachine *vm, size_t size, ObjectType type) {
	Object* object = (Object*)reallocate(vm, NULL, 0, size);
	if (object == NULL) {
		panic("[ {PANIC} Object::allocate_object] Expected to allocate memory for object, Found NULL");
	}
    object->vm = vm;
	object->type = type;
    object->vtable = &default_object_vtable;
    object->is_marked = false;

    object->next = vm->objects;
    vm->objects = object;

#if debug_print_allocations
    printf("[object::allocate_object] Allocate %zu bytes for %s\n",
           size, macro_object_type_string(type));
#endif

#if debug_log_gc
    printf("[object::allocate_object] Gc Allocate %I64d bytes for %s, pointer %p\n",
           size, macro_object_type_string(type), object);
#endif

	return object;
}

void free_object(Object* object) {
	if (object == NULL) return;

#if debug_print_allocations
    printf("[object::free_object] Free bytes for %s\n",
           macro_object_type_string(object->type));
#endif

#if debug_log_gc
    printf("[object::free_object] Gc Free %s, pointer %p\n",
           macro_object_type_string(object->type), object);
#endif

	switch (object->type) {
	case OBJ_STRING:	free_string(macro_as_string_from_obj(object)); break;
	case OBJ_FN:		free_fn(macro_as_fn_from_obj(object)); break;
	case OBJ_NATIVE:	free_native(macro_as_native_from_obj(object)); break;
	case OBJ_CLOSURE:	free_closure(macro_as_closure_from_obj(object)); break;
    case OBJ_CLASS:     free_class(macro_as_class_from_obj(object)); break;
    case OBJ_INSTANCE:  free_instance(macro_as_instance_from_obj(object)); break;
    case OBJ_BOUND_METHOD: free_bound_method(macro_as_bound_method_from_obj(object)); break;
    case OBJ_UPVALUE:	free_upvalue(macro_as_upvalue_from_obj(object)); break;
    case OBJ_STRUCT:    free_struct(macro_as_struct_from_obj(object)); break;
    case OBJ_PAIR:      free_pair(macro_as_pair_from_obj(object)); break;
    case OBJ_VEC:       free_vec(macro_as_vec_from_obj(object)); break;
    case OBJ_ENUM:      free_enum(macro_as_enum_from_obj(object)); break;
    case OBJ_ENUM_INSTANCE: free_enum_instance(macro_as_enum_instance_from_obj(object)); break;
    case OBJ_TYPE:       free_type(macro_as_type_from_obj(object)); break;
	default:            panic("[ {PANIC} Object::free_object] Unsupported object type {%d}.\n", object->type);
	}
}

void free_objects(Object *head) {
    while (head != NULL) {
        Object* next = head->next;
        free_object(head);
        head = next;
    }
}

static bool type_check(Object* object, ObjectType expected_type) {
	return object->type == expected_type;
}

bool object_equal(Object* left, Object* right) {
	if (!type_check(left, right->type)) return false;
	switch (left->type) {
	case OBJ_STRING:    return string_equal(macro_as_string_from_obj(left), macro_as_string_from_obj(right));
    case OBJ_FN:        return fn_equal(macro_as_fn_from_obj(left), macro_as_fn_from_obj(right));
    case OBJ_NATIVE:    return native_equal(macro_as_native_from_obj(left), macro_as_native_from_obj(right));
    case OBJ_CLOSURE:   return closure_equal(macro_as_closure_from_obj(left), macro_as_closure_from_obj(right));
    case OBJ_UPVALUE:   return upvalue_equal(macro_as_upvalue_from_obj(left), macro_as_upvalue_from_obj(right));
    case OBJ_CLASS:     return class_equal(macro_as_class_from_obj(left), macro_as_class_from_obj(right));
    case OBJ_INSTANCE:  return instance_equal(macro_as_instance_from_obj(left), macro_as_instance_from_obj(right));
    case OBJ_BOUND_METHOD: return bound_method_equal(macro_as_bound_method_from_obj(left), macro_as_bound_method_from_obj(right));
    case OBJ_STRUCT:    return struct_equal(macro_as_struct_from_obj(left), macro_as_struct_from_obj(right));
    case OBJ_PAIR:      return pair_equal(macro_as_pair_from_obj(left), macro_as_pair_from_obj(right));
    case OBJ_VEC:       return vec_equal(macro_as_vec_from_obj(left), macro_as_vec_from_obj(right));
    case OBJ_ENUM:      return enum_equal(macro_as_enum_from_obj(left), macro_as_enum_from_obj(right));
    case OBJ_ENUM_INSTANCE: return enum_instance_equal(macro_as_enum_instance_from_obj(left), macro_as_enum_instance_from_obj(right));
    case OBJ_TYPE:      return type_equal(macro_as_type_from_obj(left), macro_as_type_from_obj(right));
    default:            return false;
	}
}

void print_object(Object* object) {
	switch (object->type) {
	case OBJ_STRING:	print_string(macro_as_string_from_obj(object)); break;
	case OBJ_FN:	    print_fn(macro_as_fn_from_obj(object)); break;
	case OBJ_NATIVE:	print_native(macro_as_native_from_obj(object)); break;
	case OBJ_CLOSURE:   print_closure(macro_as_closure_from_obj(object)); break;
    case OBJ_CLASS:     print_class(macro_as_class_from_obj(object)); break;
    case OBJ_INSTANCE:  print_instance(macro_as_instance_from_obj(object)); break;
    case OBJ_BOUND_METHOD: print_bound_method(macro_as_bound_method_from_obj(object)); break;
    case OBJ_UPVALUE:   print_upvalue(macro_as_upvalue_from_obj(object)); break;
    case OBJ_STRUCT:    print_struct(macro_as_struct_from_obj(object)); break;
    case OBJ_PAIR:      print_pair(macro_as_pair_from_obj(object)); break;
    case OBJ_VEC:       print_vec(macro_as_vec_from_obj(object)); break;
    case OBJ_ENUM:      print_enum(macro_as_enum_from_obj(object)); break;
    case OBJ_ENUM_INSTANCE: print_enum_instance(macro_as_enum_instance_from_obj(object)); break;
    case OBJ_TYPE:      print_type(macro_as_type_from_obj(object)); break;
	default:			warning("{Warning} [print_object] Unsupported object type: %d\n", object->type);
	}
}

int snprintf_object(Object* object, char* buf, size_t size) {
    switch (object->type) {
    case OBJ_STRING:    return snprintf_string(macro_as_string_from_obj(object), buf, size);
    case OBJ_FN:        return snprintf_fn(macro_as_fn_from_obj(object), buf, size);
    case OBJ_NATIVE:    return snprintf_native(macro_as_native_from_obj(object), buf, size);
    case OBJ_CLOSURE:   return snprintf_closure(macro_as_closure_from_obj(object), buf, size);
    case OBJ_CLASS:     return snprintf_class(macro_as_class_from_obj(object), buf, size);
    case OBJ_INSTANCE:  return snprintf_instance(macro_as_instance_from_obj(object), buf, size);
    case OBJ_BOUND_METHOD: return snprintf_bound_method(macro_as_bound_method_from_obj(object), buf, size);
    case OBJ_UPVALUE:   return snprintf_upvalue(macro_as_upvalue_from_obj(object), buf, size);
    case OBJ_STRUCT:    return snprintf_struct(macro_as_struct_from_obj(object), buf, size);
    case OBJ_PAIR:      return snprintf_pair(macro_as_pair_from_obj(object), buf, size);
    case OBJ_VEC:       return snprintf_vec(macro_as_vec_from_obj(object), buf, size);
    case OBJ_ENUM:      return snprintf_enum(macro_as_enum_from_obj(object), buf, size);
    case OBJ_ENUM_INSTANCE: return snprintf_enum_instance(macro_as_enum_instance_from_obj(object), buf, size);
    case OBJ_TYPE:      return snprintf_type(macro_as_type_from_obj(object), buf, size);
    default:            warning("{Warning} [snprintf_object] Unsupported object type: %d\n", object->type);
    }
    return 0;
}

