//
// Created by Kilig on 2024/11/21.
//
#include <stdnoreturn.h>
#include <setjmp.h>
#include <stdalign.h>

#if defined(__GNUC__) && !defined(__clang__)
#define USE_COMPUTED_GOTO 1
#define LIKELY(x)       __builtin_expect(!!(x), 1)
#define UNLIKELY(x)     __builtin_expect(!!(x), 0)
#else
 #define USE_COMPUTED_GOTO 0
#define LIKELY(x)       (x)
#define UNLIKELY(x)     (x)
#endif


#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#include "vec.h"
#include "pair.h"
#include "enum.h"
#include "enum_instance.h"
#include "struct_.h"
#include "class.h"
#include "instance.h"
#include "bound_method.h"
#include "closure.h"
#include "common.h"
#include "compiler.h"
#include "error.h"
#include "fn.h"

#include "native.h"
#include "type_register.h"
#ifdef JOKER_TYPE_REGISTER_H
#include "type/include/vec.h"
#endif

#ifdef JOKER_NATIVE_H
#include "native/include/time.h"
#include "native/include/stdio.h"
#endif

#include "object.h"
#include "operator.h"
#include "string_.h"
#include "value.h"
#include "vm.h"

#if debug_trace_execution
#include "debug.h"
#endif
#if debug_enable_allocator
#include "allocator.h"
#endif


/* Virtual machine operations */
static int current_code_index(CallFrame* frame);
static void define_native(VirtualMachine* self, const char* name, NativeFnPtr function);
static bool call(VirtualMachine* self, Closure* closure, int arg_count);
static bool call_value(VirtualMachine* self, Value* callee, int arg_count);
static bool invoke(VirtualMachine* self, String* name, int arg_count);
static bool invoke_from_class(VirtualMachine* self, Class* klass, String* name, int arg_count);
static void reset_stack(VirtualMachine* self);
static InterpretResult run(VirtualMachine* self);

static InterpretResult negate(VirtualMachine* self, CallFrame* frame, Operator op);

/*computed goto table run case*/
static inline InterpretResult handle_op_pop(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_dup(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_constant(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_constant_long(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_value(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_none(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_true(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_false(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_not(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_negate(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_equal(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_not_equal(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_less(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_less_equal(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_greater(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_greater_equal(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_add(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_subtract(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_multiply(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_divide(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_mod(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_bw_and(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_bw_or(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_bw_xor(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_bw_sl(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_bw_sr(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_bw_not(VirtualMachine* self, CallFrame* frame);

static inline InterpretResult handle_op_define_global(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_get_global(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_set_global(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_get_local(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_set_local(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_get_upvalue(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_set_upvalue(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_get_property(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_set_property(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_get_super(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_get_layer_property(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_get_type(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_set_layer_property(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_jump_if_false(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_jump_if_neq(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_jump(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_loop(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_call(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_closure(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_close_upvalue(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_print(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_return(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_break(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_continue(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_match(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_class(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_method(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_inherit(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_invoke(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_super_invoke(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_struct(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_member(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_struct_inherit(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_enum(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_enum_define_member(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_enum_get_member(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_enum_member_bind(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_enum_member_match(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_layer_property_call(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_vector_new(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_vector_set(VirtualMachine* self, CallFrame* frame);
static inline InterpretResult handle_op_vector_get(VirtualMachine* self, CallFrame* frame);


// pointer arithmetic to convert stack index to uint32_t for stack access
static inline int convert_check(ptrdiff_t index) {
    if (index < 0) {
        panic("[VM] Negative stack index detected: %td (min: 0)", index);
    }
    if (index > INT32_MAX) {
        panic("[VM] Stack index overflow: %td > %td (max index)",
              index, INT32_MAX);
    }

    return (int)index;
}

void runtime_error(VirtualMachine* self, const char* message, ...) {
	va_list args;
	va_start(args, message);
	vfprintf(stderr, message, args);
	va_end(args);
	fputs("\n", stderr);

	// heap stack trace
	for (int i = self->frame_count - 1; i >= 0; i--) {
		CallFrame* frame = &self->frames[i];
		Fn* fn = frame->closure->fn;
		int index = current_code_index(frame);
		fprintf(stderr, "[line %d] in ", get_rle_line(&fn->chunk.lines, index));
		is_anonymous_fn(fn) ?
			fprintf(stderr, "script\n") :
			fprintf(stderr, "%s()\n", fn->name->chars);
	}

	// reset stack
	reset_stack(self);
}

/*
* vm->ip: 指向下一条需要执行的指令的指针
* vm->chunk->code: 指向代码段的起始地址
* vm->ip - vm->chunk->code - 1: 指向当前指令的偏移量
*/
static int current_code_index(CallFrame* frame) {
	return convert_check(frame->ip - frame->closure->fn->chunk.code - 1);
}

static bool is_falsey(VirtualMachine* self, Value* value) {
	switch (value->type) {
	case VAL_BOOL: return !value->as.boolean;
	default:
		//  raise runtime error, expected boolean for not operation, found
		runtime_error(self, "Expected boolean for not operation, Found...");
		return false;
	}
}

// 错误恢复上下文结构
typedef struct {
    jmp_buf env;
    const char* error_msg;
    int error_line;
} VmErrorContext;

// 虚拟机结构扩展
typedef struct {
    VirtualMachine base;
    VmErrorContext error_ctx;
    alignas(64) Value fast_stack[512]; // 64字节对齐快速栈
} OptimizedVM;


void init_virtual_machine(VirtualMachine* self) {
#if debug_enable_allocator
    self->allocator = new_allocator();
#endif

    // OptimizedVM* ovm = (OptimizedVM*)self;
    // memset(ovm, 0, sizeof(OptimizedVM));
    // 初始化快速栈
    // self->stack = ovm->fast_stack;
    // self->stack_top = ovm->fast_stack;

    self->gc = new_garbage_collector();

    self->objects = NULL;
	self->compiler = NULL;
    self->class_compiler = NULL;

	reset_stack(self);
	init_hashmap(&self->strings, self); // 字符串驻留
	init_hashmap(&self->globals, self); // 全局变量
    init_hashmap(&self->types, self);   // 类型

    self->tokens = NULL;
    self->tokens = new_token_list(self);

    self->init_string = NULL;
    self->init_string = new_string(self, "init", 4);    // init string

    /* register type */
    type_register(self, "Vec", &vec_vtable,vec_export_methods);

	/* register */
	define_native(self, "clock",    native_clock);
	define_native(self, "time",     native_time);
	define_native(self, "sleep",    native_sleep);
	define_native(self, "date",     native_date);
    define_native(self, "now",      native_now);

    define_native(self, "print",    native_print);
    define_native(self, "println",  native_println);
}

void free_virtual_machine(VirtualMachine* self) {
    self->init_string = NULL;
    self->class_compiler = NULL;
    free_compiler(self->compiler);

    free_hashmap(&self->strings);       // free strings internal hash table
	free_hashmap(&self->globals);       // free globals internal hash table
    free_hashmap(&self->types);         // free type internal hash table

    free_objects(self->objects);      // free all objects
    free_garbage_collector(&self->gc);  // free garbage collector

    free_tokens(self->tokens, self);
#if debug_enable_allocator
    free_allocator(self->allocator);
#endif
}

// Value stack operations
static void reset_stack(VirtualMachine* self) {
	self->stack_top = self->stack;
	self->frame_count = 0;
	self->open_upv_ptr = NULL;
}

Compiler* __attribute__((unused)) replace_compiler(VirtualMachine* self, Compiler* compiler) {
	Compiler* old_compiler = self->compiler;
	self->compiler = compiler;
	return old_compiler;
}

void push(VirtualMachine* self, Value value) {
	if (self->stack_top >= self->stack + constant_stack_max) {
		// raise overflow error, overflow the stack
		panic("[ {PANIC} VirtualMachine::push] stack overflow.");
	}
	*self->stack_top++ = value;
}

Value pop(VirtualMachine* self) {
	if (self->stack_top <= self->stack) {
		// raise underflow error, underflow the stack
		panic("[ {PANIC} VirtualMachine::pop] stack underflow.");
	}
	return *(--self->stack_top);
}

static Value* peek(VirtualMachine* self, int distance) {
	if (self->stack_top - distance < self->stack) {
		// raise underflow error, underflow the stack
		panic("[ {PANIC} VirtualMachine::peek] stack underflow.");
	}
	// return the value at the given distance from the top of the stack
	return (self->stack_top - 1 - distance);
}

/* optimization negate operation, reduce stack top pointer change, improve performance */
static InterpretResult negate(VirtualMachine* self, CallFrame* frame, Operator op) {
	if (self->stack_top <= self->stack) {
		// raise underflow error, underflow the stack
		panic("[ {PANIC} VirtualMachine::negate] stack underflow.");
	}
	Value* slot = peek(self, 0);
	switch (slot->type)
	{
	case VAL_I32: slot->as.i32 = -slot->as.i32; break;
    case VAL_I64: slot->as.i64 = -slot->as.i64; break;
    case VAL_F32: slot->as.f32 = -slot->as.f32; break;
	case VAL_F64: slot->as.f64 = -slot->as.f64; break;
	default:
        //  raise runtime error, unexpected negate operation
        runtime_error(self, "[VirtualMachine::negate]\n"
            "[line %d] where at runtime, '%s'.\n"
            "\tExpected number type, Found %s type.\n",
              get_rle_line(&frame->closure->fn->chunk.lines, current_code_index(frame)),
              macro_ops_to_string(op),
              macro_type_name_ptr(slot)
        );
		return interpret_runtime_error;
	}
	return interpret_ok;
}

static InterpretResult not_(VirtualMachine* self, CallFrame* frame, Operator op) {
	if (self->stack_top <= self->stack) {
		// raise underflow error, underflow the stack
		panic("[ {PANIC} VirtualMachine::not_] stack underflow.");
	}
	Value* slot = peek(self, 0);
	switch (slot->type)
	{
	case VAL_BOOL: slot->as.boolean = !slot->as.boolean; break;
	default:
		//  raise runtime error, expected boolean for not operation, found...
        runtime_error(self, "[VirtualMachine::negate]\n"
            "[line %d] where at runtime, '%s'.\n"
            "\tExpected boolean for not operation, Found %s type.\n",
              get_rle_line(&frame->closure->fn->chunk.lines, current_code_index(frame)),
              macro_ops_to_string(op),
              macro_type_name_ptr(slot)
        );
		return interpret_runtime_error;
	}
	return interpret_ok;
}

static void __attribute__((unused)) concatenate_string(VirtualMachine* self) {
	String* right = macro_as_string(*peek(self, 0));
	String* left = macro_as_string(*peek(self, 1));
    String* result = concat_string(&self->strings, left, right);
    pop(self);
    pop(self);
	push(self, macro_val_from_obj(result));
}

/*
* TODO: push && pop ?  => gc?
* used stack push and pop, because gc need to know the stack pointer change
*							  |  ---(-2)----> |
* value					      V				  V
* stack [..., name, function,...]  => [..., name, function,...]
*/
static void define_native(VirtualMachine* self, const char* name, NativeFnPtr function) {
	push(self, macro_val_from_obj(new_string(self, name, (int)strlen(name))));
	push(self, macro_val_from_obj(new_native(self, function, false)));
	hashmap_set(&self->globals, macro_as_string(self->stack[0]), self->stack[1]);
	// self->stack_top -= 2;	// remove the name and function from the stack
	pop(self);
	pop(self);
}

static void define_enum_member(VirtualMachine* self, String* name) {
    int store_count = macro_as_i32(*peek(self, 0));
    if (store_count < 1) {
        Enum* enum_ = macro_as_enum(peek(self, 1));
        Pair* pair = new_pair(
            self,
            macro_val_from_i32(enum_->members.count),
            macro_val_none
        );
        hashmap_set(&enum_->members, name, macro_val_from_obj(pair));
        pop(self);
    } else {
        Enum* enum_ = macro_as_enum(peek(self, 1 + store_count));
        Vec* values = new_vec(self);
        for (int i = 0; i < store_count; i++) {
            Value value = *peek(self, 1 + i);
            vec_push(values, value);
        }
        Pair* pair = new_pair(
            self,
            macro_val_from_i32(enum_->members.count),
            macro_val_from_obj(values)
        );
        hashmap_set(&enum_->members, name, macro_val_from_obj(pair));
        for(int i = 0; i < store_count; i++) {
            pop(self);
        }
        pop(self);
    }
}

static void define_member(VirtualMachine* self, String* name) {
    Value* initializer = peek(self, 0);
    Struct* struct_ = macro_as_struct(peek(self, 1));
    vec_push(struct_->names, macro_val_from_obj(name));
    hashmap_set(&struct_->fields, name, *initializer);

    struct_->count++;
    pop(self);
}

static void define_method(VirtualMachine* self, String* name) {
    Value* method = peek(self, 0);
    Class* klass = macro_as_class_from_vptr(peek(self, 1));
    hashmap_set(&klass->methods, name, *method);
    pop(self);
}

static bool bind_method(VirtualMachine* self, Class *klass, String* name) {
    Value method = hashmap_get(&klass->methods, name);

    if (macro_is_null(method)) {
        runtime_error(self, "[vm::bind_method] undefined property '%s'.", name->chars);
        return false;
    }

    BoundMethod *bound = new_bound_method(
        self,
        *peek(self, 0),
        macro_as_closure(method)
    );
    pop(self);
    push(self, macro_val_from_obj(bound));

    return true;
}

// 检查对象是否为有效属性持有者（实例或结构体）
static inline bool is_valid_property_holder(Value* value) {
    return macro_is_instance(*value) || macro_is_struct(*value);
}

// 统一属性错误报告
static void report_property_error(VirtualMachine* self, const char* format, String* name) {
    CallFrame* frame = &self->frames[self->frame_count - 1];
    int line = get_rle_line(&frame->closure->fn->chunk.lines, current_code_index(frame));
    fprintf(stderr, "[line %d] ", line);
    runtime_error(self, format, name->chars);
}

// 获取属性值的核心逻辑（实例/结构体）
static InterpretResult get_property(VirtualMachine* self, Value* holder, String* name) {
    if (macro_is_instance(*holder)) {
        Instance* instance = macro_as_instance(*holder);
        Value field_value = hashmap_get(&instance->fields, name);
        if (!macro_is_null(field_value)) {
            pop(self); // 弹出实例
            push(self, field_value);
            return interpret_ok;
        }
        // 检查方法绑定
        return bind_method(self, instance->klass, name) ? interpret_ok : interpret_runtime_error;
    } else if (macro_is_struct(*holder)) {
        Struct* struct_ = macro_as_struct(holder);
        Value field_value = hashmap_get(&struct_->fields, name);
        if (macro_is_null(field_value)) {
            report_property_error(self, "Undefined struct field '%s'.", name);
            return interpret_runtime_error;
        }
        pop(self); // 弹出结构体
        push(self, field_value);
        return interpret_ok;
    }
    return interpret_runtime_error;
}

// 设置属性值的核心逻辑（实例/结构体）
static InterpretResult set_property(VirtualMachine* self, Value* holder, String* name, Value value) {
    if (macro_is_instance(*holder)) {
        Instance* instance = macro_as_instance(*holder);
        hashmap_set(&instance->fields, name, value);
        return interpret_ok;
    } else if (macro_is_struct(*holder)) {
        Struct* struct_ = macro_as_struct(holder);
        if (!hashmap_contains_key(&struct_->fields, name)) {
            report_property_error(self, "[vm::set_property] Cannot set undefined struct field '%s'.", name);
            return interpret_runtime_error;
        }
        hashmap_set(&struct_->fields, name, value);
        return interpret_ok;
    }
    return interpret_runtime_error;
}

static bool get_layer_property(VirtualMachine* self, Value* receiver, String* name) {
    if (macro_is_obj_ptr(receiver)) {
        switch (macro_obj_ptr_type(receiver)) {
        case OBJ_ENUM: {
            Enum* enum_ = macro_as_enum(receiver);
            Value value = hashmap_get(&enum_->members, name);
            if (macro_is_null(value)) {
                runtime_error(self, "[vm::get_layer_property] undefined property '%s'.", name->chars);
                return interpret_runtime_error;
            }
            Pair* pair = macro_as_pair(value);
            EnumInstance *instance = new_enum_instance(
                    self, enum_, name, macro_as_i32(pair->first), 0);

            // receiver
            pop(self);
            push(self, macro_val_from_obj(instance));
            return true;
        }
        case OBJ_CLASS: {
            // 处理类的静态方法/属性
            Class* klass = macro_as_class_from_vptr(receiver);
            Value method = hashmap_get(&klass->methods, name);
            if (!macro_is_null(method)) {
                pop(self); // 弹出类
                push(self, method); // 推入方法
                return true;
            }
            report_property_error(self, "[vm::get_layer_property] Undefined static property '%s' in class.", name);
            return false;
        }
        default:
            report_property_error(self, "[vm::get_layer_property] Unsupported type for layer access.", name);
            return false;
        }
    } else {
        // 尝试从全局类型表中查找（如类名直接访问）
        Value type = hashmap_get(&self->types, name);
        if (!macro_is_null(type) && macro_is_class(type)) {
            pop(self); // 弹出可能的无效值
            push(self, type); // 推入类引用
            return true;
        }
        report_property_error(self, "[vm::get_layer_property] Undefined layer property '%s'.", name);
        return false;
    }
    return true;
}

static bool call_value(VirtualMachine* self, Value* callee, int arg_count) {
	if (macro_is_obj_ptr(callee)) {
		switch (macro_obj_ptr_type(callee)) {
        case OBJ_STRUCT: {
            Struct* struct_ = macro_as_struct(callee);
            if (arg_count != struct_->count){
                // raise runtime error, expected %d arguments, found...
                runtime_error(self, "[VirtualMachine::call_value] Expected %d arguments, Found %d arguments.",
                    struct_->fields.count, arg_count
                );
                return false;
            } else {
                Struct* instance = new_struct(self, struct_->name);

                for(int i = struct_->count -1; i >= 0; i--) {
                    String* name = macro_as_string(vec_get(struct_->names, i));
                    Value value = pop(self);
                    hashmap_set(&instance->fields, name, value);
                }
                self->stack_top[-1] = macro_val_from_obj(instance);
            }
            return true;
        }
        case OBJ_BOUND_METHOD: {
            BoundMethod* bound = macro_as_bound_method_from_vptr(callee);
            self->stack_top[-arg_count - 1] = bound->receiver;
            return call(self, bound->method, arg_count);
        }
        case OBJ_CLASS: {
            Class* klass = macro_as_class_from_vptr(callee);
            self->stack_top[-arg_count - 1] = macro_val_from_obj(new_instance(self, klass));

            // call the initializer
            Value initializer = hashmap_get(&klass->methods, self->init_string);
            if (!macro_is_null(initializer)) {
                // bind the initializer
                return call(self, macro_as_closure(initializer), arg_count);
            } else if (arg_count != 0) {
                // raise runtime error, expected 0 arguments, found...
                runtime_error(self, "[VirtualMachine::call_value] Expected 0 arguments, Found %d arguments.",
                              arg_count);
                return false;
            }

            // return the instance
            return true;
        }
		case OBJ_CLOSURE:
			return call(self, macro_as_closure_from_vptr(callee), arg_count);
		case OBJ_NATIVE: {
            /*
                        |<result> |	  <- arg_count -> |
                        V		  V 			      V
                [..., native_fn, arg1, arg2,..., argN, ...]
            */
			Native* native = macro_as_native_from_vptr(callee);
            NativeFnPtr native_fn = native->fn;
            int adjusted_arg_count = native->is_builtin_method ? arg_count + 1 : arg_count; // build type need to add receiver
            Value* args_start = self->stack_top - adjusted_arg_count;
            Value result = native_fn(self, adjusted_arg_count, args_start);
            self->stack_top -= arg_count + 1;  // pop args + function
            push(self, result);
            return true;
		}
		default: break;
		}
	}
	runtime_error(self, "[VirtualMachine::call_value] Can only call functions and classes.");
	return false;
}

static bool call(VirtualMachine* self, Closure* closure, int arg_count) {
	if (arg_count != closure->fn->arity) {
		runtime_error(self, "[VirtualMachine::call] Expected %d arguments, Found %d arguments.",
                      closure->fn->arity, arg_count);
		return false;
	}
	if (self->frame_count == frames_stack_max) {
		runtime_error(self, "[VirtualMachine::call] CallFrame Stack overflow.");
		return false;
	}
	CallFrame* frame = &self->frames[self->frame_count++];
	// set up the new call frame: execute func frame.
	frame->closure = closure;
	frame->ip = closure->fn->chunk.code;
	frame->slots = self->stack_top - arg_count - 1;

	return true;
}

static bool invoke(VirtualMachine* self, String* name, int arg_count) {
    Value receiver = *peek(self, arg_count);

    if (!macro_is_instance(receiver)) {
        runtime_error(self, "[VirtualMachine::invoke] Only instances have methods.");
        return false;
    }

    Instance* instance = macro_as_instance(receiver);

    // handle build type
    if (hashmap_contains_key(&self->types, instance->klass->name)) {
        Value value = hashmap_get(&instance->klass->methods, name);
        if (macro_is_null(value)) {
            runtime_error(self, "[VirtualMachine::invoke] Undefined property '%s'.", name->chars);
            return false;
        }
        return call_value(self, &value, arg_count);
    }

    // look up the method in the instance's class
    Value value = hashmap_get(&instance->fields, name);
    if (!macro_is_null(value)) {
        self->stack_top[-arg_count - 1] = value;
        return call_value(self, &value, arg_count);
    }

    return invoke_from_class(self, instance->klass, name, arg_count);
}

static bool invoke_from_class(VirtualMachine* self, Class* klass, String* name, int arg_count) {
    Value method = hashmap_get(&klass->methods, name);
    if (macro_is_null(method)) {
        runtime_error(self, "[VirtualMachine::invoke_from_class] Undefined property '%s'.", name->chars);
        return false;
    }
    return call(self, macro_as_closure(method), arg_count);
}

/* 生成支持的操作符列表 */
static void format_supported_ops(const ObjectVTable* vtable, char* buf, size_t size) {
    int pos = 0;
    pos += snprintf(buf + pos, size - pos, "Supported operators: ");
    for (int i = 0; i < BINARY_COUNT; ++i) {
        if (vtable->binary_operators[i]) {
            pos += snprintf(buf + pos, size - pos, "%s ", macro_ops_to_string(i));
        }
    }
    for(int i = 0; i < UNARY_COUNT; ++i){
        if(vtable->unary_operators[i]) {
            pos += snprintf(buf + pos, size - pos, "%s ", macro_ops_to_string(i));
        }
    }
}

/* 统一类型提升 */
static void coerce_types(VirtualMachine* vm, Value* lhs, Value* rhs, Operator op) {
    // 数值类型自动提升
    if (macro_is_number(*lhs) && macro_is_number(*rhs)) {
        numeric_type_promotion(lhs, rhs);
        return;
    }

    // 数值与字符串相加时自动转换为字符串
    if (op == ADD) {
        if (macro_is_number(*lhs) && macro_is_obj_ptr(rhs) &&
            macro_as_obj_ptr(rhs)->vtable == &string_vtable) {
            String* string = number_to_string(vm, lhs);
            macro_stored_string(lhs, string);
            return;
        }
        if (macro_is_number(*rhs) && macro_is_obj_ptr(lhs) &&
            macro_as_obj_ptr(lhs)->vtable == &string_vtable) {
            String* string = number_to_string(vm, rhs);
            macro_stored_string(rhs, string);
            return;
        }
    }
}

static InterpretResult handle_binary_i32_op(VirtualMachine* vm, Operator op, Value* lhs, Value* rhs) {
    int32_t a = macro_as_i32(*lhs);
    int32_t b = macro_as_i32(*rhs);

    switch (op) {
        case ADD: MACRO_CHECK_I32_OVERFLOW(a, +, b); macro_set_i32(lhs, a + b); pop(vm); return interpret_ok;
        case SUB: MACRO_CHECK_I32_OVERFLOW(a, -, b); macro_set_i32(lhs, a - b); pop(vm); return interpret_ok;
        case MUL: MACRO_CHECK_I32_OVERFLOW(a, *, b); macro_set_i32(lhs, a * b); pop(vm); return interpret_ok;
        case DIV:
            if (b == 0) {
                runtime_error(vm, "Division by zero");
                return interpret_runtime_error;
            }
            macro_set_i32(lhs, a / b);
            pop(vm);
            return interpret_ok;
        case MOD:
            if (b == 0) {
                runtime_error(vm, "Modulo by zero");
                return interpret_runtime_error;
            }
            macro_set_i32(lhs, a % b);
            pop(vm);
            return interpret_ok;
        case EQ: macro_set_bool(lhs, a == b); pop(vm); return interpret_ok;
        case NEQ:macro_set_bool(lhs, a != b); pop(vm); return interpret_ok;
        case GT: macro_set_bool(lhs, a > b); pop(vm); return interpret_ok;
        case LT: macro_set_bool(lhs, a < b); pop(vm); return interpret_ok;
        case GTE:macro_set_bool(lhs, a >= b); pop(vm); return interpret_ok;
        case LTE:macro_set_bool(lhs, a <= b); pop(vm); return interpret_ok;
        case SHL:macro_set_i32(lhs, a << b); pop(vm); return interpret_ok;
        case SHR:macro_set_i32(lhs, a >> b); pop(vm); return interpret_ok;
        case BIT_XOR:macro_set_i32(lhs, a ^ b); pop(vm); return interpret_ok;
        case BIT_OR: macro_set_i32(lhs, a | b); pop(vm); return interpret_ok;
        case BIT_AND:macro_set_i32(lhs, a & b); pop(vm); return interpret_ok;
        default: return MACRO_OP_NOT_SUPPORTED("i32", op);
    }
}

static InterpretResult handle_binary_i64_op(VirtualMachine* vm, Operator op, Value* lhs, Value* rhs) {
    int64_t a = macro_as_i64(*lhs);
    int64_t b = macro_as_i64(*rhs);

    switch (op) {
        case ADD: macro_set_i64(lhs, a + b); pop(vm); return interpret_ok;
        case SUB: macro_set_i64(lhs, a - b); pop(vm); return interpret_ok;
        case MUL: macro_set_i64(lhs, a * b); pop(vm); return interpret_ok;
        case DIV:
            if (b == 0) {
                runtime_error(vm, "Division by zero");
                return interpret_runtime_error;
            }
            macro_set_i64(lhs, a / b);
            pop(vm);
            return interpret_ok;
        case MOD:
            if (b == 0) {
                runtime_error(vm, "Modulo by zero");
                return interpret_runtime_error;
            }
            macro_set_i64(lhs, a % b);
            pop(vm);
            return interpret_ok;
        case EQ:  macro_set_bool(lhs, a == b); pop(vm); return interpret_ok;
        case NEQ: macro_set_bool(lhs, a != b); pop(vm); return interpret_ok;
        case GT:  macro_set_bool(lhs, a > b);  pop(vm); return interpret_ok;
        case LT:  macro_set_bool(lhs, a < b);  pop(vm); return interpret_ok;
        case GTE: macro_set_bool(lhs, a >= b); pop(vm); return interpret_ok;
        case LTE: macro_set_bool(lhs, a <= b); pop(vm); return interpret_ok;
        case SHL: macro_set_i64(lhs, a << b); pop(vm); return interpret_ok;
        case SHR: macro_set_i64(lhs, a >> b); pop(vm); return interpret_ok;
        case BIT_AND: macro_set_i64(lhs, a & b); pop(vm); return interpret_ok;
        case BIT_OR:  macro_set_i64(lhs, a | b); pop(vm); return interpret_ok;
        case BIT_XOR: macro_set_i64(lhs, a ^ b); pop(vm); return interpret_ok;
        default:
            return MACRO_OP_NOT_SUPPORTED("i64", op);
    }
}

static InterpretResult handle_binary_f32_op(VirtualMachine* vm, Operator op, Value* lhs, Value* rhs) {
    float a = macro_as_f32(*lhs);
    float b = macro_as_f32(*rhs);

    switch (op) {
        case ADD: macro_set_f32(lhs, a + b); pop(vm); return interpret_ok;
        case SUB: macro_set_f32(lhs, a - b); pop(vm); return interpret_ok;
        case MUL: macro_set_f32(lhs, a * b); pop(vm); return interpret_ok;
        case DIV:
            if (b == 0.0f) {
                runtime_error(vm, "Division by zero");
                return interpret_runtime_error;
            }
            macro_set_f32(lhs, a / b);
            pop(vm);
            return interpret_ok;
        case EQ:  macro_set_bool(lhs, a == b); pop(vm); return interpret_ok;
        case NEQ: macro_set_bool(lhs, a != b); pop(vm); return interpret_ok;
        case GT:  macro_set_bool(lhs, a > b);  pop(vm); return interpret_ok;
        case LT:  macro_set_bool(lhs, a < b);  pop(vm); return interpret_ok;
        case GTE: macro_set_bool(lhs, a >= b); pop(vm); return interpret_ok;
        case LTE: macro_set_bool(lhs, a <= b); pop(vm); return interpret_ok;
        default:  return MACRO_OP_NOT_SUPPORTED("f32", op);
    }
}

static InterpretResult handle_binary_f64_op(VirtualMachine* vm, Operator op, Value* lhs, Value* rhs) {
    double a = macro_as_f64(*lhs);
    double b = macro_as_f64(*rhs);

    switch (op) {
        case ADD: macro_set_f64(lhs, a + b); pop(vm); return interpret_ok;
        case SUB: macro_set_f64(lhs, a - b); pop(vm); return interpret_ok;
        case MUL: macro_set_f64(lhs, a * b); pop(vm); return interpret_ok;
        case DIV:
            if (b == 0) {
                runtime_error(vm, "Division by zero");
                return interpret_runtime_error;
            }
            macro_set_f64(lhs, a / b);
            pop(vm);
            return interpret_ok;
        case MOD: macro_set_f64(lhs, fmod(a, b)); pop(vm); return interpret_ok;
        case EQ:  macro_set_bool(lhs, a == b); pop(vm); return interpret_ok;
        case NEQ: macro_set_bool(lhs, a != b); pop(vm); return interpret_ok;
        case GT:  macro_set_bool(lhs, a > b);  pop(vm); return interpret_ok;
        case LT:  macro_set_bool(lhs, a < b);  pop(vm); return interpret_ok;
        case GTE: macro_set_bool(lhs, a >= b); pop(vm); return interpret_ok;
        case LTE: macro_set_bool(lhs, a <= b); pop(vm); return interpret_ok;
        default: return MACRO_OP_NOT_SUPPORTED("f64", op);
    }
}

static InterpretResult handle_binary_bool_op(VirtualMachine* vm, Operator op, Value* lhs, Value* rhs) {
    bool a = macro_as_bool(*lhs);
    bool b = macro_as_bool(*rhs);

    switch (op) {
        case EQ:  macro_set_bool(lhs, a == b); pop(vm); return interpret_ok;
        case NEQ: macro_set_bool(lhs, a != b); pop(vm); return interpret_ok;
        case AND: macro_set_bool(lhs, a && b); pop(vm); return interpret_ok;
        case OR:  macro_set_bool(lhs, a || b); pop(vm); return interpret_ok;
        default:  return MACRO_OP_NOT_SUPPORTED("bool", op);
    }
}

static InterpretResult handle_binary_none_op(VirtualMachine* vm, Operator op, Value* lhs, Value* rhs) {
    switch (op) {
        case EQ: macro_set_bool(lhs, macro_is_none(*rhs)); pop(vm);return interpret_ok;
        case NEQ: macro_set_bool(lhs, !macro_is_none(*rhs)); pop(vm);return interpret_ok;
        default:
            runtime_error(vm, "Operation '%s' not supported for None type",
                          macro_ops_to_string(op));
            return interpret_runtime_error;
    }
}

/*弹栈right ,修改left*/
static InterpretResult read_binary(VirtualMachine* self, CallFrame* frame, Operator op) {
    Value* rhs = peek(self, 0);
    Value* rhs_orig = rhs;
    Value* lhs = peek(self, 1);

    // 参数校验
    if (!rhs || !lhs || !MACRO_IN_BINARY_OP(op)) {
        runtime_error(self, "Invalid operands or operator");
        return interpret_runtime_error;
    }

    // 类型强制转换
    coerce_types(self, lhs, rhs, op);

    // 优先处理对象类型
    if (macro_is_obj(*lhs)) {
        Object* obj = macro_as_obj_ptr(lhs);
        const ObjectVTable* vtable = obj->vtable;

        // 动态分派操作符
        BinaryOpHandler handler = vtable->binary_operators[op] ?
                                  vtable->binary_operators[op] :
                                  default_object_vtable.binary_operators[op];
        InterpretResult result = handler(lhs, rhs);
        if (result == interpret_ok) {
            pop(self);  // 弹出右操作数, 左操作数保留，为操作后的数值
            return result;
        }

        // 生成详细的错误信息
        char supported_ops[256];
        format_supported_ops(vtable, supported_ops, sizeof(supported_ops));
        runtime_error(self,
              "[Line %d] Operator '%s' not supported for %s. %s",
              get_rle_line(&frame->closure->fn->chunk.lines, current_code_index(frame)),
              macro_ops_to_string(op),
              vtable->type_name,
              supported_ops
        );
        return interpret_runtime_error;
    }

    // 处理基本类型
    switch (lhs->type) {
        case VAL_I32: return handle_binary_i32_op(self, op, lhs, rhs);
        case VAL_I64: return handle_binary_i64_op(self, op, lhs, rhs);
        case VAL_F32: return handle_binary_f32_op(self, op, lhs, rhs);
        case VAL_F64: return handle_binary_f64_op(self, op, lhs, rhs);
        case VAL_BOOL: return handle_binary_bool_op(self, op, lhs, rhs);
        case VAL_NONE: return handle_binary_none_op(self, op, lhs, rhs);
        default: break;
    }

    // 未知类型错误
    runtime_error(self,
          "[Line %d] Unsupported operand types: %s %s %s",
          get_rle_line(&frame->closure->fn->chunk.lines, current_code_index(frame)),
          macro_type_name(*lhs),
          macro_ops_to_string(op),
          macro_type_name(*rhs_orig)
    );
    return interpret_runtime_error;
}

static InterpretResult handle_unary_i32_op(VirtualMachine* self, Operator op, Value* operand) {
    switch (op) {
        case NEG:
            operand->as.i32 = -operand->as.i32;
            return interpret_ok;
        case BIT_NOT:
            operand->as.i32 = ~operand->as.i32;
            return interpret_ok;
        default:
            runtime_error(self, "Invalid unary operator '%s' for i32",macro_ops_to_string(op));
            return interpret_runtime_error;
    }
}

static InterpretResult handle_unary_i64_op(VirtualMachine* self, Operator op, Value* operand) {
    switch (op) {
        case NEG:
            operand->as.i64 = -operand->as.i64;
            return interpret_ok;
        case BIT_NOT:
            operand->as.i64 = ~operand->as.i64;
            return interpret_ok;
        default:
            runtime_error(self, "Invalid unary operator '%s' for i64", macro_ops_to_string(op));
            return interpret_runtime_error;
    }
}

static InterpretResult handle_unary_f32_op(VirtualMachine* self, Operator op, Value* operand) {
    switch (op) {
        case NEG:
            operand->as.f32 = -operand->as.f32;
            return interpret_ok;
        default:
            runtime_error(self, "Invalid unary operator '%s' for f32", macro_ops_to_string(op));
            return interpret_runtime_error;
    }
}

static InterpretResult handle_unary_f64_op(VirtualMachine* self, Operator op, Value* operand) {
    switch (op) {
        case NEG:
            operand->as.f64 = -operand->as.f64;
            return interpret_ok;
        default:
            runtime_error(self, "Invalid unary operator '%s' for f64", macro_ops_to_string(op));
            return interpret_runtime_error;
    }
}

static InterpretResult handle_unary_bool_op(VirtualMachine* self, Operator op, Value* operand) {
    switch (op) {
        case NOT:
            operand->as.boolean = !operand->as.boolean;
            return interpret_ok;
        default:
            runtime_error(self, "Invalid unary operator '%s' for bool", macro_ops_to_string(op));
            return interpret_runtime_error;
    }
}

static InterpretResult handle_unary_none_op(VirtualMachine* self, Operator op, Value* operand) {
    (void)operand;

    runtime_error(self, "Cannot apply unary operator '%s' to None", macro_ops_to_string(op));
    return interpret_runtime_error;
}

/*不弹栈，直接修改 operand */
static InterpretResult read_unary(VirtualMachine *self, CallFrame *frame, Operator op) {
    Value* operand = peek(self, 0);

    if (!operand || !MACRO_IN_UNARY_OP(op)) {
        runtime_error(self, "Invalid operands or operator");
        return interpret_runtime_error;
    }

    if (macro_is_obj(*operand)) {
        Object* obj = macro_as_obj_ptr(operand);
        const ObjectVTable* vtable = obj->vtable;
        UnaryOpHandler handler = vtable->unary_operators[MACRO_UNARY_INDEX(op)] ?
                                 vtable->unary_operators[MACRO_UNARY_INDEX(op)] :
                                 default_object_vtable.unary_operators[MACRO_UNARY_INDEX(op)];
        InterpretResult result = handler(operand);
        if (result == interpret_ok) {
            return result;
        }
        // 生成详细的错误信息
        char supported_ops[256];
        format_supported_ops(vtable, supported_ops, sizeof(supported_ops));
        runtime_error(self,
              "[Line %d] Operator '%s' not supported for %s. %s",
              get_rle_line(&frame->closure->fn->chunk.lines, current_code_index(frame)),
              macro_ops_to_string(op),
              vtable->type_name,
              supported_ops
        );
        return interpret_runtime_error;
    }

    switch (operand->type) {
        case VAL_I32: return handle_unary_i32_op(self, op, operand);
        case VAL_I64: return handle_unary_i64_op(self, op, operand);
        case VAL_F32: return handle_unary_f32_op(self, op, operand);
        case VAL_F64: return handle_unary_f64_op(self, op, operand);
        case VAL_NONE: return handle_unary_none_op(self, op, operand);
        case VAL_BOOL: return handle_unary_bool_op(self, op, operand);
        default: break;
    }

    runtime_error(self,
          "[Line %d] Unsupported operand types: %s %s",
          get_rle_line(&frame->closure->fn->chunk.lines, current_code_index(frame)),
          macro_type_name(*operand),
          macro_ops_to_string(op)
    );

    return interpret_runtime_error;
}


/*
* Interprets the given chunk of code.
* Returns an InterpretResult indicating the result of the interpretation.
*/
InterpretResult interpret(VirtualMachine* self, const char* source) {
    Fn* fn = compile(self, source);
    if (fn == NULL) return interpret_compile_error;

    push(self, macro_val_from_obj(fn));
    Closure* closure = new_closure(fn);
    pop(self);
    push(self, macro_val_from_obj(closure));
    call(self, closure, 0);					// call the top-level function closure

    return run(self);
}





// 安全栈操作宏
#define MACRO_STACK_GUARD(cond, msg) do { \
    if (UNLIKELY(!(cond))) { \
        vm_panic(self, "[LINE %d] %s", \
            current_code_index(frame), (msg)); \
    } \
} while(0)

#define MACRO_SAFE_PUSH(value) do { \
    MACRO_STACK_GUARD(self->stack_top < self->stack + constant_stack_max, "Stack overflow"); \
    *self->stack_top++ = (value); \
} while(0)

#define MACRO_SAFE_POP() ({ \
    MACRO_STACK_GUARD(self->stack_top > self->stack, "Stack underflow"); \
    *--self->stack_top; \
})


// SIMD优化宏（x86 SSE）
#ifdef __SSE2__
#include <emmintrin.h>
#define VECTOR_LOAD(ptr) _mm_load_si128((const __m128i*)(ptr))
#define VECTOR_STORE(ptr, val) _mm_store_si128((__m128i*)(ptr), (val))
#else
#define VECTOR_LOAD(ptr) (*(ptr))
#define VECTOR_STORE(ptr, val) (*(ptr) = (val))
#endif


noreturn void vm_panic(VirtualMachine* self, const char* fmt, ...) {
    OptimizedVM* ovm = (OptimizedVM*)self;
    va_list args;
    va_start(args, fmt);

    // 保存错误上下文
    vsnprintf(
        (char*)ovm->error_ctx.error_msg,
        sizeof(ovm->error_ctx.error_msg),
        fmt,
        args
    );
    ovm->error_ctx.error_line = current_code_index(&self->frames[self->frame_count-1]);

    va_end(args);
    longjmp(ovm->error_ctx.env, 1);

    abort();
}



static InterpretResult run(VirtualMachine* self) {
    // OptimizedVM* ovm = (OptimizedVM*)self;

    CallFrame* frame = &self->frames[self->frame_count - 1];

// 错误恢复点
//    if (setjmp(ovm->error_ctx.env)) {
//        return interpret_runtime_error;
//    }

#if USE_COMPUTED_GOTO
#define OP_LABEL(op) LABEL_##op:
#define OP_DISPATCH() goto *op_table[*frame->ip++]

// ========== GCC计算跳转实现 ==========
// 定义所有指令标签的跳转表
    static const void* op_table[] = {
            // 使用 && 获取标签地址（GCC扩展）
            &&LABEL_op_pop,
            &&LABEL_op_dup,
            &&LABEL_op_constant,
            &&LABEL_op_constant_long,
            &&LABEL_op_value,
            &&LABEL_op_none,
            &&LABEL_op_true,
            &&LABEL_op_false,
            &&LABEL_op_not,
            &&LABEL_op_negate,
            &&LABEL_op_equal,
            &&LABEL_op_not_equal,
            &&LABEL_op_less,
            &&LABEL_op_less_equal,
            &&LABEL_op_greater,
            &&LABEL_op_greater_equal,
            &&LABEL_op_add,
            &&LABEL_op_subtract,
            &&LABEL_op_multiply,
            &&LABEL_op_divide,
            &&LABEL_op_mod,
            &&LABEL_op_bw_and,
            &&LABEL_op_bw_or,
            &&LABEL_op_bw_xor,
            &&LABEL_op_bw_sl,
            &&LABEL_op_bw_sr,
            &&LABEL_op_bw_not,
            &&LABEL_op_define_global,
            &&LABEL_op_get_global,
            &&LABEL_op_set_global,
            &&LABEL_op_get_local,
            &&LABEL_op_set_local,
            &&LABEL_op_get_upvalue,
            &&LABEL_op_set_upvalue,
            &&LABEL_op_get_property,
            &&LABEL_op_set_property,
            &&LABEL_op_get_super,
            &&LABEL_op_get_layer_property,
            &&LABEL_op_get_type,
            &&LABEL_op_jump_if_false,
            &&LABEL_op_jump_if_neq,
            &&LABEL_op_jump,
            &&LABEL_op_loop,
            &&LABEL_op_call,
            &&LABEL_op_closure,
            &&LABEL_op_close_upvalue,
            &&LABEL_op_print,
            &&LABEL_op_return,
            &&LABEL_op_break,
            &&LABEL_op_continue,
            &&LABEL_op_match,
            &&LABEL_op_class,
            &&LABEL_op_method,
            &&LABEL_op_inherit,
            &&LABEL_op_invoke,
            &&LABEL_op_super_invoke,
            &&LABEL_op_struct,
            &&LABEL_op_member,
            &&LABEL_op_struct_inherit,
            &&LABEL_op_enum,
            &&LABEL_op_enum_define_member,
            &&LABEL_op_enum_get_member,
            &&LABEL_op_enum_member_bind,
            &&LABEL_op_enum_member_match,
            &&LABEL_op_layer_property_call,
            &&LABEL_op_vector_new,
            &&LABEL_op_vector_set,
            &&LABEL_op_vector_get,
            &&LABEL_UNKNOWN_OPCODE,
    };

    /* computed goto table*/
    OP_DISPATCH();


    OP_LABEL(op_pop) {
        handle_op_pop(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_dup) {
        handle_op_dup(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_constant) {
        handle_op_constant(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_constant_long) {
        handle_op_constant_long(self, frame);
        OP_DISPATCH();
    }

    OP_LABEL(op_value) {
        handle_op_value(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_none) {
        handle_op_none(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_true) {
        handle_op_true(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_false) {
        handle_op_false(self, frame);
        OP_DISPATCH();
    }

    OP_LABEL(op_not) {
        handle_op_not(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_negate) {
        handle_op_negate(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_equal) {
        handle_op_equal(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_not_equal) {
        handle_op_not_equal(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_less) {
        handle_op_less(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_less_equal) {
        handle_op_less_equal(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_greater) {
        handle_op_greater(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_greater_equal) {
        handle_op_greater_equal(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_add) {
        handle_op_add(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_subtract) {
        handle_op_subtract(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_multiply) {
        handle_op_multiply(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_divide) {
        handle_op_divide(self, frame);
        OP_DISPATCH();
    }

    OP_LABEL(op_mod) {
        handle_op_mod(self, frame);
        OP_DISPATCH();
    }

    OP_LABEL(op_bw_and) {
        handle_op_bw_and(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_bw_or) {
        handle_op_bw_or(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_bw_xor) {
        handle_op_bw_xor(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_bw_sl) {
        handle_op_bw_sl(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_bw_sr) {
        handle_op_bw_sr(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_bw_not) {
        handle_op_bw_not(self, frame);
        OP_DISPATCH();
    }

    OP_LABEL(op_define_global) {
        handle_op_define_global(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_get_global) {
        handle_op_get_global(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_set_global) {
        handle_op_set_global(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_get_local) {
        handle_op_get_local(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_set_local) {
        handle_op_set_local(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_get_upvalue) {
        handle_op_get_upvalue(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_set_upvalue) {
        handle_op_set_upvalue(self, frame);
        OP_DISPATCH();
    }

    OP_LABEL(op_get_property) {
        handle_op_get_property(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_set_property) {
        handle_op_set_property(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_get_super) {
        handle_op_get_super(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_get_layer_property) {
        handle_op_get_layer_property(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_get_type) {
        handle_op_get_type(self, frame);
        OP_DISPATCH();
    }

    OP_LABEL(op_jump_if_false) {
        handle_op_jump_if_false(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_jump_if_neq) {
        handle_op_jump_if_neq(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_jump) {
        handle_op_jump(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_loop) {
        handle_op_loop(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_call) {
        handle_op_call(self, frame);
        frame = &self->frames[self->frame_count - 1];
        OP_DISPATCH();
    }

    OP_LABEL(op_closure) {
        handle_op_closure(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_close_upvalue) {
        handle_op_close_upvalue(self, frame);
        OP_DISPATCH();
    }

    OP_LABEL(op_print) {
        handle_op_print(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_return) {
        if(handle_op_return(self, frame) == interpret_ok) {
            return interpret_ok;
        }
        frame = &self->frames[self->frame_count - 1];
        OP_DISPATCH();
    }
    OP_LABEL(op_break) {
        handle_op_break(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_continue) {
        handle_op_continue(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_match) {
        handle_op_match(self, frame);
        OP_DISPATCH();
    }

    OP_LABEL(op_class) {
        handle_op_class(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_method) {
        handle_op_method(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_inherit) {
        handle_op_inherit(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_invoke) {
        handle_op_invoke(self, frame);
        frame = &self->frames[self->frame_count - 1];
        OP_DISPATCH();
    }
    OP_LABEL(op_super_invoke) {
        handle_op_super_invoke(self, frame);
        frame = &self->frames[self->frame_count - 1];
        OP_DISPATCH();

    }
    OP_LABEL(op_struct) {
        handle_op_struct(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_member) {
        handle_op_member(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_struct_inherit) {
        handle_op_struct_inherit(self, frame);
        OP_DISPATCH();
    }

    OP_LABEL(op_enum) {
        handle_op_enum(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_enum_define_member) {
        handle_op_enum_define_member(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_enum_get_member) {
        handle_op_enum_get_member(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_enum_member_bind) {
        handle_op_enum_member_bind(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_enum_member_match) {
        handle_op_enum_member_match(self, frame);
        OP_DISPATCH();
    }

    OP_LABEL(op_layer_property_call) {
        handle_op_layer_property_call(self, frame);
        frame = &self->frames[self->frame_count - 1];
        OP_DISPATCH();
    }
    OP_LABEL(op_vector_new) {
        handle_op_vector_new(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_vector_set) {
        handle_op_vector_set(self, frame);
        OP_DISPATCH();
    }
    OP_LABEL(op_vector_get) {
        handle_op_vector_get(self, frame);
        OP_DISPATCH();
    }

    LABEL_UNKNOWN_OPCODE: {
        vm_panic(self,
                 "Unknown opcode 0x%02X at IP=%td (offset %d)",
                 *frame->ip,
                 frame->ip - frame->closure->fn->chunk.code,
                 current_code_index(frame)
        );
        // 明确告知编译器此代码不可达
        __builtin_unreachable();
    }

#else
/* read a byte from the current instruction pointer: vm->ip++ goto frame->ip++ */
#define macro_read_byte() (*frame->ip++)
/* read a byte from the current instruction pointer: vm->ip++ goto frame->ip++ give it to constant long*/
// #define macro_read_byte_long() ((*frame->ip++) << 8 | (*frame->ip++))
#define macro_read_byte_long()                  \
    ({                                          \
        uint8_t high = *frame->ip++;            \
        uint8_t low = *frame->ip++;             \
        (uint16_t)((high << 8) | low);          \
    })
/* read a constant from the constant pool */
#define macro_read_constant() (frame->closure->fn->chunk.constants.values[macro_read_byte()])
/* read a constant long from the constant pool */
#define macro_read_constant_long() (frame->closure->fn->chunk.constants.values[macro_read_byte_long()])
/* read a short from the current instruction pointer */
#define macro_runtime_error_raised(result)			\
    do {											\
        if (result == interpret_runtime_error) {	\
            return result;							\
        }											\
    } while (false)
/* read a string from the constant pool */
#define macro_read_string() (macro_as_string(macro_read_constant()))
/* read a short from the current instruction pointer */
#define macro_read_short()											\
	(frame->ip +=2,													\
	((uint16_t)(frame->ip[-2]) << 8) | (uint16_t)(frame->ip[-1]))


    Value constant;

    while (true) {

/* debug_trace_execution */
#if debug_trace_execution
        /* stack trace: print the current stack all at once.
		*
		VirtualMachine:
			Value* stack_top = vm->stack_top;
			Value* stack = vm->stack;
			for (int i = 0; i < constant_stack_max; i++) {
		*/
		printf("		");
		for (Value* slot = self->stack; slot < self->stack_top; slot++) {
			printf("[ ");
			print_value(*slot);
			printf(" ]");
		}
		printf("\n");

		/* print the current instruction pointer.
		*
		VirtualMachine:
			chunk -> chunk->code [first instruction]
			ip -> current instruction pointer
		offset:
			vm->ip - vm->chunk->code
		*/
		disassemble_instruction(&frame->closure->fn->chunk, (int)(frame->ip - frame->closure->fn->chunk.code));
#endif
        /* execute the current instruction.
        *
        VirtualMachine:
            push(vm, constant);			// push constant to stack
            pop(vm);					// pop constant from stack
        */
        uint8_t instruction = macro_read_byte();
        switch (instruction) {
            case op_constant: {
                constant = macro_read_constant();
                push(self, constant);
                break;
            }
            case op_constant_long: {
                constant = macro_read_constant_long();
                push(self, constant);
                break;
            }
            case op_value:  push(self, macro_val_from_i32(macro_read_byte())); break;
            case op_negate: macro_runtime_error_raised(negate(self, frame, NEG)); break;
            case op_not:    macro_runtime_error_raised(not_(self, frame, NOT)); break;
            case op_none:	push(self, macro_val_none); break;
            case op_true:	push(self, macro_val_from_bool(true)); break;
            case op_false:	push(self, macro_val_from_bool(false)); break;
            case op_dup:    push(self, *peek(self, 0)); break;
            case op_equal: {
                Value right = pop(self);
                Value left = pop(self);
                push(self, macro_val_from_bool(values_equal(left, right)));
                break;
            }
            case op_not_equal: {
                Value right = pop(self);
                Value left = pop(self);
                push(self, macro_val_from_bool(!values_equal(left, right)));
                break;
            }
            case op_greater:		macro_runtime_error_raised(read_binary(self, frame, GT)); break;
            case op_greater_equal:	macro_runtime_error_raised(read_binary(self, frame, GTE)); break;
            case op_less:			macro_runtime_error_raised(read_binary(self, frame, LT)); break;
            case op_less_equal:		macro_runtime_error_raised(read_binary(self, frame, LTE)); break;
            case op_add: {
                Value* r_slot = peek(self, 0);
                Value* l_slot = peek(self, 1);
                macro_check_nullptr(r_slot);
                macro_check_nullptr(l_slot);
                if (macro_is_string(*l_slot) && macro_is_string(*r_slot)) {
                    concatenate_string(self);
                }
                else { macro_runtime_error_raised(read_binary(self, frame, ADD)); } break;
            }
            case op_subtract:		macro_runtime_error_raised(read_binary(self, frame, SUB)); break;
            case op_multiply:		macro_runtime_error_raised(read_binary(self, frame, MUL)); break;
            case op_divide:			macro_runtime_error_raised(read_binary(self, frame, DIV)); break;
            case op_bw_and:         macro_runtime_error_raised(read_binary(self, frame, BIT_AND)); break;
            case op_bw_or:          macro_runtime_error_raised(read_binary(self, frame, BIT_OR)); break;
            case op_bw_xor:         macro_runtime_error_raised(read_binary(self, frame, BIT_XOR)); break;
            case op_bw_not:         macro_runtime_error_raised(read_unary(self, frame, BIT_NOT)); break;
            case op_bw_sl:          macro_runtime_error_raised(read_binary(self, frame, SHL)); break;
            case op_bw_sr:          macro_runtime_error_raised(read_binary(self, frame, SHR)); break;
            case op_pop: pop(self); break;
            case op_define_global: {
                String* identifier = macro_read_string();
                hashmap_set(&self->globals, identifier, *peek(self, 0));
                pop(self);
                break;
            }
            case op_set_global: {
                String* identifier = macro_read_string();
                Entry* entry = hashmap_get_entry(&self->globals, identifier);
                if (is_empty_entry(entry)) {
                    runtime_error(self, "[op_set_global | line %d] where: at runtime undefined global variable '%s'.",
                                  get_rle_line(&frame->closure->fn->chunk.lines, current_code_index(frame)),
                                  identifier->chars
                    );
                    return interpret_runtime_error;
                }
                else {
                    entry->value = *peek(self, 0);
                }
                break;
            }
            case op_get_global: {
                String* identifier = macro_read_string();
                Value value = hashmap_get(&self->globals, identifier);
                if (macro_is_null(value)) {
                    runtime_error(self, "[op_get_global | line %d] where: at runtime undefined global variable '%s'.",
                                  get_rle_line(&frame->closure->fn->chunk.lines, current_code_index(frame)),
                                  identifier->chars
                    );
                    return interpret_runtime_error;
                }
                push(self, value);
                break;
            }
            case op_set_local: {
                uint8_t slot = macro_read_byte();
                frame->slots[slot] = *peek(self, 0);
                break;
            }
            case op_get_local: {
                uint8_t slot = macro_read_byte();
                Value value = frame->slots[slot];
                if (macro_is_null(value)) {
                    runtime_error(self, "[line %d] where: at runtime undefined local variable '%s'.",
                                  get_rle_line(&frame->closure->fn->chunk.lines, current_code_index(frame)),
                                  slot
                    );
                    return interpret_runtime_error;
                }
                push(self, value);
                break;
            }
            case op_get_upvalue: {
                uint8_t slot = macro_read_byte();
                push(self, *(frame->closure->upvalue_ptrs[slot]->location));
                break;
            }
            case op_set_upvalue: {
                uint8_t slot = macro_read_byte();
                *frame->closure->upvalue_ptrs[slot]->location = *peek(self, 0);
                break;
            }
            case op_get_property: {
                String* name = macro_read_string();
                Value* holder = peek(self, 0); // 栈顶是属性持有者

                if (!is_valid_property_holder(holder)) {
                    runtime_error(self, "[op_get_property] Property access on non-object type.");
                    return interpret_runtime_error;
                }

                if (get_property(self, holder, name) != interpret_ok) {
                    return interpret_runtime_error;
                }
                break;
            }
            case op_set_property: {
                String* name = macro_read_string();
                Value value  = *peek(self, 0);      // 待设置的值
                Value* holder = peek(self, 1);      // 属性持有者（实例或结构体）

                if (!is_valid_property_holder(holder)) {
                    runtime_error(self, "[op_set_property] Cannot set property on non-object type.");
                    return interpret_runtime_error;
                }

                InterpretResult result = set_property(self, holder, name, value);
                if (result != interpret_ok) {
                    return result;
                }

                // 调整栈：弹出设置的值和持有者，重新推入值以保持表达式结果
                pop(self); // 弹出值
                pop(self); // 弹出持有者
                push(self, value);
                break;
            }
            case op_get_super: {
                String* method_name = macro_read_string();
                Class* super_class = macro_as_class(pop(self));

                if (!bind_method(self, super_class, method_name)) {
                    return interpret_runtime_error;
                }
                break;
            }
            case op_jump_if_false: {
                // short: 16-bit unsigned signed integer
                uint16_t offset = macro_read_short();

                if (is_falsey(self, peek(self, 0))) {
                    frame->ip += offset;
                }
                break;
            }
            case op_jump_if_neq: {
                Value pattern = pop(self);        // pattern
                Value matched = pop(self);        // matched

                if (!values_equal(matched, pattern)) {
                    int offset = macro_read_short();
                    frame->ip += offset;
                } else {
                    frame->ip += 2; // skip the offset
                }

                push(self, matched);
                break;
            }
            case op_jump: {
                uint16_t offset = macro_read_short();
                frame->ip += offset;
                break;
            }
            case op_loop: {
                uint16_t offset = macro_read_short();
                frame->ip -= offset;
                break;
            }
            case op_match: {
                // pass match jump
                (void)macro_read_short();
                break;
            }
            case op_call: {
                int arg_count = macro_read_byte();
                // call success value can in frames insert new frame(function call).
                if (!call_value(self, peek(self, arg_count), arg_count)) {
                    return interpret_runtime_error;
                }
                // update current frame pointer point to the new frame(function call);
                // ps: set base pointer
                frame = &self->frames[self->frame_count - 1];
                break;
            }
            case op_invoke: {
                String* method_name = macro_read_string();
                int arg_count = macro_read_byte();

                if (!invoke(self, method_name, arg_count)) {
                    return interpret_runtime_error;
                }

                frame = &self->frames[self->frame_count - 1];
                break;
            }
            case op_super_invoke: {
                String* method = macro_read_string();
                int arg_count = macro_read_byte();
                Class* super_class = macro_as_class(pop(self));

                if (!invoke_from_class(self, super_class, method, arg_count)) {
                    runtime_error(self, "[line %d] where: at runtime undefined superclass method '%s'.",
                                  get_rle_line(
                                          &frame->closure->fn->chunk.lines,
                                          current_code_index(frame)
                                  ),
                                  method->chars
                    );
                    return interpret_runtime_error;
                }
                frame = &self->frames[self->frame_count - 1];
                break;
            }
            case op_break: {
                uint16_t offset = macro_read_short();
                frame->ip += offset;
                break;
            }
            case op_continue: {
                uint16_t offset = macro_read_short();
                frame->ip -= offset;
                break;
            }
            case op_class: {
                push(self, macro_val_from_obj(new_class(self, macro_read_string())));
                break;
            }
            case op_inherit: {
                // TODO: inherit class.
                Value superclass = *peek(self, 1);
                if (!macro_is_class(superclass)) {
                    runtime_error(self, "[line %d] where: at runtime superclass must be a class.",
                                  get_rle_line(
                                          &frame->closure->fn->chunk.lines,
                                          current_code_index(frame)
                                  )
                    );
                    return interpret_runtime_error;
                }

                Class* klass = macro_as_class_from_vptr(peek(self, 0));
                hashmap_add_all(&macro_as_class(superclass)->methods, &klass->methods);
                pop(self);  // pop klass
                break;
            }
            case op_method: {
                define_method(self, macro_read_string());
                break;
            }
            case op_closure: {
                Fn* fn = macro_as_fn(macro_read_constant());
                Closure* closure = new_closure(fn);
                push(self, macro_val_from_obj(closure));

                for (int i = 0; i < closure->upvalue_count; i++) {
                    upvalue_info_t upv_info = macro_read_byte();
                    bool is_local = upv_info >> 7;
                    uint8_t index = upv_info & 0b01111111;
                    if (is_local) {
                        closure->upvalue_ptrs[i] = capture_upvalue(self,frame->slots + index);
                    }
                    else {
                        closure->upvalue_ptrs[i] = frame->closure->upvalue_ptrs[index];
                    }
                }
                break;
            }
            case op_close_upvalue: {
                close_upvalues(self->open_upv_ptr, self->stack_top - 1);
                pop(self);
                break;
            }
            case op_struct: {
                push(self, macro_val_from_obj(new_struct(self, macro_read_string())));
                break;
            }
            case op_struct_inherit: {
                Value* value = peek(self, 1);
                if (!macro_is_struct(*value)){
                    runtime_error(self, "[line %d] where: at runtime super must be a struct.",
                                  get_rle_line(
                                          &frame->closure->fn->chunk.lines,
                                          current_code_index(frame)
                                  )
                    );
                    return interpret_runtime_error;
                }
                Struct* super_ = macro_as_struct(value);
                Struct* struct_ = macro_as_struct(peek(self, 0));
                hashmap_add_all(&super_->fields, &struct_->fields);
                struct_->count = super_->count;

//            for(int i = 0; i < super_->count; i++) {
//                struct_->names[i] = super_->names[i];
//            }

                pop(self); // pop struct
                break;
            }
            case op_member: {
                define_member(self, macro_read_string());
                break;
            }
            case op_enum: {
                push(self, macro_val_from_obj(new_enum(self, macro_read_string())));
                break;
            }
            case op_enum_define_member: {
                define_enum_member(self, macro_read_string());
                break;
            }
            case op_enum_get_member: {
                push(self, macro_val_from_obj(macro_read_string()));
                break;
            }
            case op_enum_member_match: {
                String* member = macro_as_string(pop(self)); // member
                Value pattern = pop(self);                  // pattern  enum
                Value matched = pop(self);                  // matched  instance
                push(self, matched);

                if (string_equal(macro_as_enum_instance_from_value(matched)->name, member)
                    && enum_equal(macro_as_enum_from_value(pattern), macro_as_enum_instance_from_value(matched)->enum_type)) {
                    frame->ip += 2; // skip the offset
                    EnumInstance* instance = macro_as_enum_instance_from_value(matched);
                    if (!enum_values_is_empty(instance)) {
                        size_t store_count = vec_len(instance->values);
                        for(size_t i = 0; i < store_count; i++) {
                            push(self, vec_get(macro_as_enum_instance_from_value(matched)->values, i));
                        }
                    }
                } else {
                    int offset = macro_read_short();
                    frame->ip += offset;
                }
                break;
            }
            case op_enum_member_bind: {
                uint8_t bind_count = macro_read_byte();
                for(int i = 0; i < bind_count; i++) {
                    uint8_t index = macro_read_byte();
                    frame->slots[index] = *peek(self, bind_count - i - 1);
                }
                break;
            }
            case op_layer_property_call: {
                String* layer_member = macro_read_string();
                int arg_count = macro_read_byte();
                Value* value = peek(self, arg_count);

                if (macro_is_class(*value)) {
                    // 类静态方法调用：ClassName::method(args)
                    Class* klass = macro_as_class(*value);
                    Value method = hashmap_get(&klass->methods, layer_member);
                    if (macro_is_null(method)) {
                        runtime_error(self, "Undefined static method '%s'.", layer_member->chars);
                        return interpret_runtime_error;
                    }
                    // 替换栈中的类为方法，准备调用
                    self->stack_top[-arg_count - 1] = method;

                    // call success value can in frames insert new frame(function call).
                    if (!call_value(self, &method, arg_count)) {
                        return interpret_runtime_error;
                    }

                    // update current frame pointer point to the new frame(function call);
                    // ps: set base pointer
                    frame = &self->frames[self->frame_count - 1];
                    break;
                } else if (macro_is_enum(*value)) {
                    Enum* enum_ = macro_as_enum(value);
                    Value member = hashmap_get(&enum_->members, layer_member);
                    Pair* pair = macro_as_pair(member);

                    EnumInstance *instance = new_enum_instance(
                            self,
                            macro_as_enum(value),
                            layer_member,
                            macro_as_i32(pair->first),
                            arg_count
                    );
                    for(int i = arg_count - 1; i >= 0; i--) {
                        vec_set(instance->values, i, pop(self));
                    }

                    pop(self);
                    push(self, macro_val_from_obj(instance));    // TODO: enum instance
                    break;
                }
                break;
            }
            case op_get_layer_property: {
                Value* value = peek(self, 0);
                String* layer_name = macro_read_string();
                if (!get_layer_property(self, value, layer_name)) {
                    return interpret_runtime_error;
                }
                break;
            }
            case op_get_type: {
                String* identifier = macro_read_string();
                Value type = hashmap_get(&self->types, identifier);
                if (macro_is_null(type)) {
                    runtime_error(self, "[op_get_type | line %d] where: at runtime undefined global type variable '%s'.",
                                  get_rle_line(&frame->closure->fn->chunk.lines, current_code_index(frame)),
                                  identifier->chars
                    );
                    return interpret_runtime_error;
                }
                push(self, type);
                break;
            }
            case op_vector_new: {
                uint8_t  element_count = macro_read_byte();
                Vec* vec = new_vec_with_capacity(self, element_count);

                for (int i = element_count - 1; i >= 0; i--) {
                    Value element = pop(self);
                    vec->start[i] = element;
                    vec->finish++;
                }

                Class* vector_class = macro_as_class(hashmap_get(
                        &self->types,
                        new_string(self, "Vec", 3)
                )
                );
                Instance* vector_instance = new_instance(self, vector_class);
                hashmap_set(
                        &vector_instance->fields,
                        new_string(self, "_data", 5),
                        macro_val_from_obj(vec)
                );

                push(self, macro_val_from_obj(vector_instance));
                break;
            }
            case op_vector_get: {
                Value index_val = pop(self);
                Value vec_val = pop(self);

                if (!macro_is_instance(vec_val) && strcmp(macro_as_instance(vec_val)->klass->name->chars, "Vec") != 0) {
                    runtime_error(self, "[line %d] Expected vector for 'op_vector_get', Found %s.",
                                  get_rle_line(&frame->closure->fn->chunk.lines, current_code_index(frame)),
                                  macro_type_name(vec_val)
                    );
                    return interpret_runtime_error;
                }
                if (!macro_is_i32(index_val)) {
                    runtime_error(self, "[line %d] Expected integer index for 'op_vector_get', Found %s.",
                                  get_rle_line(&frame->closure->fn->chunk.lines, current_code_index(frame)),
                                  macro_type_name(index_val)
                    );
                    return interpret_runtime_error;
                }

                Instance* vec_instance = macro_as_instance(vec_val);
                Vec* vec = macro_as_vec(hashmap_get(
                        &vec_instance->fields, new_string(self, "_data", 5)));
                int32_t index = macro_as_i32(index_val);

                if (index < 0 || index >= (int32_t)vec_len(vec)) {
                    runtime_error(self, "[line %d] Vector index out of bounds (size=%d, index=%d).",
                                  get_rle_line(&frame->closure->fn->chunk.lines, current_code_index(frame)),
                                  vec_len(vec), index
                    );
                    return interpret_runtime_error;
                }

                push(self, vec_get(vec, index));
                break;
            }
            case op_vector_set: {
                // 从栈顶弹出值、索引、向量对象
                Value value = pop(self);
                Value index_val = pop(self);
                Value vec_val = pop(self);

                // 类型检查
                if (!macro_is_instance(vec_val) && strcmp(macro_as_instance(vec_val)->klass->name->chars, "Vec") != 0) {
                    runtime_error(self, "[line %d] Expected vector for 'op_vector_get', Found %s.",
                                  get_rle_line(&frame->closure->fn->chunk.lines, current_code_index(frame)),
                                  macro_type_name(vec_val)
                    );
                    return interpret_runtime_error;
                }
                if (!macro_is_i32(index_val)) {
                    runtime_error(self, "[line %d] Expected integer index for 'op_vector_get', Found %s.",
                                  get_rle_line(&frame->closure->fn->chunk.lines, current_code_index(frame)),
                                  macro_type_name(index_val)
                    );
                    return interpret_runtime_error;
                }

                Instance* vec_instance = macro_as_instance(vec_val);
                Vec* vec = macro_as_vec(hashmap_get(
                        &vec_instance->fields, new_string(self, "_data", 5)));
                int32_t index = macro_as_i32(index_val);

                // 索引越界检查
                if (index < 0 || index >= (int32_t)vec_len(vec)) {
                    runtime_error(self, "[line %d] Vector index out of bounds (size=%d, index=%d).",
                                  get_rle_line(&frame->closure->fn->chunk.lines, current_code_index(frame)),
                                  vec_len(vec), index
                    );
                    return interpret_runtime_error;
                }

                // 更新向量中的值
                vec_set(vec, index, value);
                push(self, value); // 将设置的值重新压栈（可选，根据语义需求）
                break;
            }
            case op_return: {
                Value result = pop(self);	// pop the return value
                close_upvalues(self->open_upv_ptr, frame->slots);
                self->frame_count--;		// jump to the caller frame
                if (self->frame_count == 0) {
                    return interpret_ok;
                }

                // update vm stack top pointer point to the caller frame stack top pointer.
                // this doesn't need set frame ip pointer,
                // because caller frame execute frame->ip over
                // can return to caller frame continue to execute caller->frame->ip++, next.
                self->stack_top = frame->slots;
                // push the return value to the caller frame stack.
                push(self, result);
                // update frame pointer point to the caller frame.
                frame = &self->frames[self->frame_count - 1];
                break;
            }
            case op_print: {
                printf_value(pop(self)); break;
            }
            default: panic("[ {PANIC} VirtualMachine::run] Expected run command arm, Found noting arm!");
        }
    }

#undef macro_read_short
#undef macro_read_string
#undef macro_runtime_error_raised
#undef macro_read_constant_long
#undef macro_read_constant
#undef macro_read_byte_long
#undef macro_read_byte
#endif
}


#define macro_read_byte(frame) (*frame->ip++)
#define macro_read_byte_long(frame)             \
    ({                                          \
        uint8_t high = *frame->ip++;            \
        uint8_t low = *frame->ip++;             \
        (uint16_t)((high << 8) | low);          \
    })
#define macro_read_constant(frame) (frame->closure->fn->chunk.constants.values[macro_read_byte(frame)])
#define macro_read_constant_long(frame) (frame->closure->fn->chunk.constants.values[macro_read_byte_long(frame)])
#define macro_runtime_error_raised(result)			\
    do {											\
        if (result == interpret_runtime_error) {	\
            return result;							\
        }											\
    } while (false)
#define macro_read_string(frame) (macro_as_string(macro_read_constant(frame)))
#define macro_read_short(frame)											\
	(frame->ip +=2,													    \
	((uint16_t)(frame->ip[-2]) << 8) | (uint16_t)(frame->ip[-1]))


static inline InterpretResult handle_op_pop(VirtualMachine* self, CallFrame* frame){
    (void)frame;

    pop(self);
    return interpret_ok;
}
static inline InterpretResult handle_op_dup(VirtualMachine* self, CallFrame* frame){
    (void)frame;

    push(self, *peek(self, 0));
    return interpret_ok;
}
static inline InterpretResult handle_op_constant(VirtualMachine* self, CallFrame* frame){
    push(self,macro_read_constant(frame));
    return interpret_ok;
}
static inline InterpretResult handle_op_constant_long(VirtualMachine* self, CallFrame* frame){
    push(self, macro_read_constant_long(frame));
    return interpret_ok;
}
static inline InterpretResult handle_op_value(VirtualMachine* self, CallFrame* frame){
    push(self, macro_val_from_i32(macro_read_byte(frame)));
    return interpret_ok;
}
static inline InterpretResult handle_op_none(VirtualMachine* self, CallFrame* frame){
    (void)frame;

    push(self, macro_val_none);
    return interpret_ok;
}
static inline InterpretResult handle_op_true(VirtualMachine* self, CallFrame* frame){
    (void)frame;

    push(self, macro_val_from_bool(true));
    return interpret_ok;
}
static inline InterpretResult handle_op_false(VirtualMachine* self, CallFrame* frame){
    (void)frame;

    push(self, macro_val_from_bool(false));
    return interpret_ok;
}
static inline InterpretResult handle_op_not(VirtualMachine* self, CallFrame* frame){
    return not_(self, frame, NOT);
}
static inline InterpretResult handle_op_negate(VirtualMachine* self, CallFrame* frame){
    return negate(self, frame, NEG);
}
static inline InterpretResult handle_op_equal(VirtualMachine* self, CallFrame* frame){
    return read_binary(self, frame, EQ);
}
static inline InterpretResult handle_op_not_equal(VirtualMachine* self, CallFrame* frame){
    return read_binary(self, frame, NEQ);
}
static inline InterpretResult handle_op_less(VirtualMachine* self, CallFrame* frame){
    return read_binary(self, frame, LT);
}
static inline InterpretResult handle_op_less_equal(VirtualMachine* self, CallFrame* frame){
    return read_binary(self, frame, LTE);
}
static inline InterpretResult handle_op_greater(VirtualMachine* self, CallFrame* frame){
    return read_binary(self, frame, GT);
}
static inline InterpretResult handle_op_greater_equal(VirtualMachine* self, CallFrame* frame){
    return read_binary(self, frame, GTE);
}
static inline InterpretResult handle_op_add(VirtualMachine* self, CallFrame* frame){
    return read_binary(self, frame, ADD);
}
static inline InterpretResult handle_op_subtract(VirtualMachine* self, CallFrame* frame){
    return read_binary(self, frame, SUB);
}
static inline InterpretResult handle_op_multiply(VirtualMachine* self, CallFrame* frame){
    return read_binary(self, frame, MUL);
}
static inline InterpretResult handle_op_divide(VirtualMachine* self, CallFrame* frame){
    return read_binary(self, frame, DIV);
}
static inline InterpretResult handle_op_mod(VirtualMachine* self, CallFrame* frame){
    return read_binary(self, frame, MOD);
}
static inline InterpretResult handle_op_bw_and(VirtualMachine* self, CallFrame* frame){
    return read_binary(self, frame, BIT_AND);
}
static inline InterpretResult handle_op_bw_or(VirtualMachine* self, CallFrame* frame){
    return read_binary(self, frame, BIT_OR);
}
static inline InterpretResult handle_op_bw_xor(VirtualMachine* self, CallFrame* frame){
    return read_binary(self, frame, BIT_XOR);
}
static inline InterpretResult handle_op_bw_sl(VirtualMachine* self, CallFrame* frame){
    return read_binary(self, frame, SHL);
}
static inline InterpretResult handle_op_bw_sr(VirtualMachine* self, CallFrame* frame){
    return read_binary(self, frame, SHR);
}
static inline InterpretResult handle_op_bw_not(VirtualMachine* self, CallFrame* frame){
    return read_unary(self, frame, BIT_NOT);
}


static inline InterpretResult handle_op_define_global(VirtualMachine* self, CallFrame* frame){
    String* identifier = macro_read_string(frame);
    hashmap_set(&self->globals, identifier, *peek(self, 0));
    pop(self);
    return interpret_ok;
}
static inline InterpretResult handle_op_get_global(VirtualMachine* self, CallFrame* frame){
    String* identifier = macro_read_string(frame);
    Value value = hashmap_get(&self->globals, identifier);
    if (macro_is_null(value)) {
        runtime_error(self, "[op_get_global | line %d] where: at runtime undefined global variable '%s'.",
              get_rle_line(&frame->closure->fn->chunk.lines, current_code_index(frame)),
              identifier->chars
        );
        return interpret_runtime_error;
    }
    push(self, value);
    return interpret_ok;
}
static inline InterpretResult handle_op_set_global(VirtualMachine* self, CallFrame* frame){
    String* identifier = macro_read_string(frame);
    Entry* entry = hashmap_get_entry(&self->globals, identifier);
    if (is_empty_entry(entry)) {
        runtime_error(self, "[op_set_global | line %d] where: at runtime undefined global variable '%s'.",
              get_rle_line(&frame->closure->fn->chunk.lines, current_code_index(frame)),
              identifier->chars
        );
        return interpret_runtime_error;
    }
    else {
        entry->value = *peek(self, 0);
    }
    return interpret_ok;
}
static inline InterpretResult handle_op_get_local(VirtualMachine* self, CallFrame* frame){
    uint8_t slot = macro_read_byte(frame);
    Value value = frame->slots[slot];
    if (macro_is_null(value)) {
        runtime_error(self, "[line %d] where: at runtime undefined local variable '%s'.",
              get_rle_line(&frame->closure->fn->chunk.lines, current_code_index(frame)),
              slot
        );
        return interpret_runtime_error;
    }
    push(self, value);
    return interpret_ok;
}
static inline InterpretResult handle_op_set_local(VirtualMachine* self, CallFrame* frame){
    uint8_t slot = macro_read_byte(frame);
    frame->slots[slot] = *peek(self, 0);
    return interpret_ok;
}
static inline InterpretResult handle_op_get_upvalue(VirtualMachine* self, CallFrame* frame){
    uint8_t slot = macro_read_byte(frame);
    push(self, *(frame->closure->upvalue_ptrs[slot]->location));
    return interpret_ok;
}
static inline InterpretResult handle_op_set_upvalue(VirtualMachine* self, CallFrame* frame){
    uint8_t slot = macro_read_byte(frame);
    *frame->closure->upvalue_ptrs[slot]->location = *peek(self, 0);
    return interpret_ok;
}
static inline InterpretResult handle_op_get_property(VirtualMachine* self, CallFrame* frame){
    String* name = macro_read_string(frame);
    Value* holder = peek(self, 0); // 栈顶是属性持有者

    if (!is_valid_property_holder(holder)) {
        runtime_error(self, "[op_get_property] Property access on non-object type.");
        return interpret_runtime_error;
    }

    if (get_property(self, holder, name) != interpret_ok) {
        return interpret_runtime_error;
    }
    return interpret_ok;
}
static inline InterpretResult handle_op_set_property(VirtualMachine* self, CallFrame* frame){
    String* name = macro_read_string(frame);
    Value value  = *peek(self, 0);      // 待设置的值
    Value* holder = peek(self, 1);      // 属性持有者（实例或结构体）

    if (!is_valid_property_holder(holder)) {
        runtime_error(self, "[op_set_property] Cannot set property on non-object type.");
        return interpret_runtime_error;
    }

    InterpretResult result = set_property(self, holder, name, value);
    if (result != interpret_ok) {
        return result;
    }

    // 调整栈：弹出设置的值和持有者，重新推入值以保持表达式结果
    pop(self); // 弹出值
    pop(self); // 弹出持有者
    push(self, value);
    return interpret_ok;
}
static inline InterpretResult handle_op_get_super(VirtualMachine* self, CallFrame* frame){
    String* method_name = macro_read_string(frame);
    Class* super_class = macro_as_class(pop(self));

    if (!bind_method(self, super_class, method_name)) {
        return interpret_runtime_error;
    }
    return interpret_ok;
}
static inline InterpretResult handle_op_get_layer_property(VirtualMachine* self, CallFrame* frame){
    Value* value = peek(self, 0);
    String* layer_name = macro_read_string(frame);
    if (!get_layer_property(self, value, layer_name)) {
        return interpret_runtime_error;
    }
    return interpret_ok;
}
static inline InterpretResult handle_op_get_type(VirtualMachine* self, CallFrame* frame){
    String* identifier = macro_read_string(frame);
    Value type = hashmap_get(&self->types, identifier);
    if (macro_is_null(type)) {
        runtime_error(self, "[op_get_type | line %d] where: at runtime undefined global type variable '%s'.",
          get_rle_line(&frame->closure->fn->chunk.lines, current_code_index(frame)),
          identifier->chars
        );
        return interpret_runtime_error;
    }
    push(self, type);
    return interpret_ok;
}

__attribute__((unused))
static inline InterpretResult handle_op_set_layer_property(VirtualMachine* self, CallFrame* frame){
    (void)self;
    (void)frame;
    return interpret_ok;
}
static inline InterpretResult handle_op_jump_if_false(VirtualMachine* self, CallFrame* frame){
    // short: 16-bit unsigned signed integer
    uint16_t offset = macro_read_short(frame);

    if (is_falsey(self, peek(self, 0))) {
        frame->ip += offset;
    }
    return interpret_ok;
}
static inline InterpretResult handle_op_jump_if_neq(VirtualMachine* self, CallFrame* frame){
    Value pattern = pop(self);        // pattern
    Value matched = pop(self);        // matched

    if (!values_equal(matched, pattern)) {
        int offset = macro_read_short(frame);
        frame->ip += offset;
    } else {
        frame->ip += 2; // skip the offset
    }

    push(self, matched);
    return interpret_ok;
}
static inline InterpretResult handle_op_jump(VirtualMachine* self, CallFrame* frame){
    (void)self;

    uint16_t offset = macro_read_short(frame);
    frame->ip += offset;
    return interpret_ok;
}
static inline InterpretResult handle_op_loop(VirtualMachine* self, CallFrame* frame){
    (void)self;

    uint16_t offset = macro_read_short(frame);
    frame->ip -= offset;
    return interpret_ok;
}
static inline InterpretResult handle_op_call(VirtualMachine* self, CallFrame* frame){
    int arg_count = macro_read_byte(frame);
    // call success value can in frames insert new frame(function call).
    if (!call_value(self, peek(self, arg_count), arg_count)) {
        return interpret_runtime_error;
    }
    // update current frame pointer point to the new frame(function call);
    // ps: set base pointer
    frame = &self->frames[self->frame_count - 1];
    return interpret_ok;
}
static inline InterpretResult handle_op_closure(VirtualMachine* self, CallFrame* frame){
    Fn* fn = macro_as_fn(macro_read_constant(frame));
    Closure* closure = new_closure(fn);
    push(self, macro_val_from_obj(closure));

    for (int i = 0; i < closure->upvalue_count; i++) {
        upvalue_info_t upv_info = macro_read_byte(frame);
        bool is_local = upv_info >> 7;
        uint8_t index = upv_info & 0b01111111;
        if (is_local) {
            closure->upvalue_ptrs[i] = capture_upvalue(self,frame->slots + index);
        }
        else {
            closure->upvalue_ptrs[i] = frame->closure->upvalue_ptrs[index];
        }
    }
    return interpret_ok;
}
static inline InterpretResult handle_op_close_upvalue(VirtualMachine* self, CallFrame* frame){
    (void)frame;

    close_upvalues(self->open_upv_ptr, self->stack_top - 1);
    pop(self);
    return interpret_ok;
}
static inline InterpretResult handle_op_print(VirtualMachine* self, CallFrame* frame){
    (void)frame;

    printf_value(pop(self));
    return interpret_ok;
}
static inline InterpretResult handle_op_return(VirtualMachine* self, CallFrame* frame){
    Value result = pop(self);	// pop the return value
    close_upvalues(self->open_upv_ptr, frame->slots);
    self->frame_count--;		// jump to the caller frame
    if (self->frame_count == 0) {
        return interpret_ok;
    }

    // update vm stack top pointer point to the caller frame stack top pointer.
    // this doesn't need set frame ip pointer,
    // because caller frame execute frame->ip over
    // can return to caller frame continue to execute caller->frame->ip++, next.
    self->stack_top = frame->slots;
    // push the return value to the caller frame stack.
    push(self, result);
    // update frame pointer point to the caller frame.
    frame = &self->frames[self->frame_count - 1];
    return interpret_passed;
}
static inline InterpretResult handle_op_break(VirtualMachine* self, CallFrame* frame){
    (void)self;

    uint16_t offset = macro_read_short(frame);
    frame->ip += offset;
    return interpret_ok;
}
static inline InterpretResult handle_op_continue(VirtualMachine* self, CallFrame* frame){
    (void)self;

    uint16_t offset = macro_read_short(frame);
    frame->ip -= offset;
    return interpret_ok;
}
static inline InterpretResult handle_op_match(VirtualMachine* self, CallFrame* frame){
    (void)self;

    // pass match jump
    (void)macro_read_short(frame);
    return interpret_ok;
}
static inline InterpretResult handle_op_class(VirtualMachine* self, CallFrame* frame){
    push(self, macro_val_from_obj(new_class(self, macro_read_string(frame))));
    return interpret_ok;
}
static inline InterpretResult handle_op_method(VirtualMachine* self, CallFrame* frame){
    define_method(self, macro_read_string(frame));
    return interpret_ok;
}
static inline InterpretResult handle_op_inherit(VirtualMachine* self, CallFrame* frame){
    // TODO: inherit class.
    Value superclass = *peek(self, 1);
    if (!macro_is_class(superclass)) {
        runtime_error(self, "[line %d] where: at runtime superclass must be a class.",
          get_rle_line(
                  &frame->closure->fn->chunk.lines,
                  current_code_index(frame)
          )
        );
        return interpret_runtime_error;
    }

    Class* klass = macro_as_class_from_vptr(peek(self, 0));
    hashmap_add_all(&macro_as_class(superclass)->methods, &klass->methods);
    pop(self);  // pop klass
    return interpret_ok;
}
static inline InterpretResult handle_op_invoke(VirtualMachine* self, CallFrame* frame){
    String* method_name = macro_read_string(frame);
    int arg_count = macro_read_byte(frame);

    if (!invoke(self, method_name, arg_count)) {
        return interpret_runtime_error;
    }

    frame = &self->frames[self->frame_count - 1];
    return interpret_ok;
}
static inline InterpretResult handle_op_super_invoke(VirtualMachine* self, CallFrame* frame){
    String* method = macro_read_string(frame);
    int arg_count = macro_read_byte(frame);
    Class* super_class = macro_as_class(pop(self));

    if (!invoke_from_class(self, super_class, method, arg_count)) {
        runtime_error(self, "[line %d] where: at runtime undefined superclass method '%s'.",
                      get_rle_line(
                              &frame->closure->fn->chunk.lines,
                              current_code_index(frame)
                      ),
                      method->chars
        );
        return interpret_runtime_error;
    }
    frame = &self->frames[self->frame_count - 1];
    return interpret_ok;
}
static inline InterpretResult handle_op_struct(VirtualMachine* self, CallFrame* frame){
    String* struct_name = macro_read_string(frame);
    push(self, macro_val_from_obj(new_struct(self, struct_name)));
    return interpret_ok;
}
static inline InterpretResult handle_op_member(VirtualMachine* self, CallFrame* frame) {
    String *member_name = macro_read_string(frame);
    define_member(self, member_name);
    return interpret_ok;
}
static inline InterpretResult handle_op_struct_inherit(VirtualMachine* self, CallFrame* frame){
    Value* value = peek(self, 1);
    if (!macro_is_struct(*value)){
        runtime_error(self, "[line %d] where: at runtime super must be a struct.",
          get_rle_line(
                  &frame->closure->fn->chunk.lines,
                  current_code_index(frame)
          )
        );
        return interpret_runtime_error;
    }
    Struct* super_ = macro_as_struct(value);
    Struct* struct_ = macro_as_struct(peek(self, 0));
    hashmap_add_all(&super_->fields, &struct_->fields);
    vec_extend(struct_->names, super_->names);
    struct_->count = super_->count;

    pop(self); // pop struct
    return interpret_ok;
}
static inline InterpretResult handle_op_enum(VirtualMachine* self, CallFrame* frame){
    push(self, macro_val_from_obj(new_enum(self, macro_read_string(frame))));
    return interpret_ok;
}
static inline InterpretResult handle_op_enum_define_member(VirtualMachine* self, CallFrame* frame){
    define_enum_member(self, macro_read_string(frame));
    return interpret_ok;
}
static inline InterpretResult handle_op_enum_get_member(VirtualMachine* self, CallFrame* frame){
    push(self, macro_val_from_obj(macro_read_string(frame)));
    return interpret_ok;
}
static inline InterpretResult handle_op_enum_member_bind(VirtualMachine* self, CallFrame* frame){
    uint8_t bind_count = macro_read_byte(frame);
    for(int i = 0; i < bind_count; i++) {
        uint8_t index = macro_read_byte(frame);
        frame->slots[index] = *peek(self, bind_count - i - 1);
    }
    return interpret_ok;
}
static inline InterpretResult handle_op_enum_member_match(VirtualMachine* self, CallFrame* frame){
    String* member = macro_as_string(pop(self)); // member
    Value pattern = pop(self);                  // pattern  enum
    Value matched = pop(self);                  // matched  instance
    push(self, matched);

    if (string_equal(macro_as_enum_instance_from_value(matched)->name, member)
        && enum_equal(macro_as_enum_from_value(pattern), macro_as_enum_instance_from_value(matched)->enum_type)) {
        frame->ip += 2; // skip the offset
        EnumInstance* instance = macro_as_enum_instance_from_value(matched);
        if (!enum_values_is_empty(instance)) {
            size_t store_count = vec_len(instance->values);
            for(size_t i = 0; i < store_count; i++) {
                push(self, vec_get(macro_as_enum_instance_from_value(matched)->values, i));
            }
        }
    } else {
        int offset = macro_read_short(frame);
        frame->ip += offset;
    }
    return interpret_ok;
}
static inline InterpretResult handle_op_layer_property_call(VirtualMachine* self, CallFrame* frame){
    String* layer_member = macro_read_string(frame);
    int arg_count = macro_read_byte(frame);
    Value* value = peek(self, arg_count);

    if (macro_is_class(*value)) {
        // 类静态方法调用：ClassName::method(args)
        Class* klass = macro_as_class(*value);
        Value method = hashmap_get(&klass->methods, layer_member);
        if (macro_is_null(method)) {
            runtime_error(self, "Undefined static method '%s'.", layer_member->chars);
            return interpret_runtime_error;
        }
        // 替换栈中的类为方法，准备调用
        self->stack_top[-arg_count - 1] = method;

        // call success value can in frames insert new frame(function call).
        if (!call_value(self, &method, arg_count)) {
            return interpret_runtime_error;
        }

        // update current frame pointer point to the new frame(function call);
        // ps: set base pointer
        frame = &self->frames[self->frame_count - 1];

    } else if (macro_is_enum(*value)) {
        Enum* enum_ = macro_as_enum(value);
        Value member = hashmap_get(&enum_->members, layer_member);
        Pair* pair = macro_as_pair(member);

        EnumInstance *instance = new_enum_instance(
                self,
                macro_as_enum(value),
                layer_member,
                macro_as_i32(pair->first),
                arg_count
        );
        for(int i = arg_count - 1; i >= 0; i--) {
            vec_set(instance->values, i, pop(self));
        }

        pop(self);
        push(self, macro_val_from_obj(instance));    // TODO: enum instance
    }
    return interpret_ok;
}
static inline InterpretResult handle_op_vector_new(VirtualMachine* self, CallFrame* frame){
    uint8_t  element_count = macro_read_byte(frame);
    Vec* vec = new_vec_with_capacity(self, element_count);

    for (int i = element_count - 1; i >= 0; i--) {
        Value element = pop(self);
        vec->start[i] = element;
        vec->finish++;
    }

    Class* vector_class = type_find(self, "Vec");
    Instance* vector_instance = new_instance(self, vector_class);
    hashmap_set(
            &vector_instance->fields,
            new_string(self, "_data", 5),
            macro_val_from_obj(vec)
    );

    push(self, macro_val_from_obj(vector_instance));
    return interpret_ok;
}
static inline InterpretResult handle_op_vector_set(VirtualMachine* self, CallFrame* frame){
    Value value = pop(self);
    Value index_val = pop(self);
    Value vec_val = pop(self);

    // 类型检查
    if (UNLIKELY(!macro_is_instance(vec_val))
        && strcmp(macro_as_instance(vec_val)->klass->name->chars, "Vec") != 0) {
        runtime_error(self, "[line %d] Expected vector for 'op_vector_get', Found %s.",
          get_rle_line(&frame->closure->fn->chunk.lines, current_code_index(frame)),
          macro_type_name(vec_val)
        );
        return interpret_runtime_error;
    }
    if (UNLIKELY(!macro_is_i32(index_val))) {
        runtime_error(self, "[line %d] Expected integer index for 'op_vector_get', Found %s.",
          get_rle_line(&frame->closure->fn->chunk.lines, current_code_index(frame)),
          macro_type_name(index_val)
        );
        return interpret_runtime_error;
    }

    Instance* vec_instance = macro_as_instance(vec_val);
    Vec* vec = macro_as_vec(hashmap_get(
            &vec_instance->fields, new_string(self, "_data", 5)));
    int32_t index = macro_as_i32(index_val);

    if (UNLIKELY(index < 0 || index >= (int32_t)vec_len(vec))) {
        runtime_error(self, "[line %d] Vector index out of bounds (size=%d, index=%d).",
          get_rle_line(&frame->closure->fn->chunk.lines, current_code_index(frame)),
          vec_len(vec), index
        );
        return interpret_runtime_error;
    }

    vec_set(vec, index, value);
    push(self, value);
    return interpret_ok;
}
static inline InterpretResult handle_op_vector_get(VirtualMachine* self, CallFrame* frame){
    Value index_val = pop(self);
    Value vec_val = pop(self);

    if (UNLIKELY(!macro_is_instance(vec_val))
        && strcmp(macro_as_instance(vec_val)->klass->name->chars, "Vec") != 0) {
        runtime_error(self, "[line %d] Expected vector for 'op_vector_get', Found %s.",
          get_rle_line(&frame->closure->fn->chunk.lines, current_code_index(frame)),
          macro_type_name(vec_val)
        );
        return interpret_runtime_error;
    }
    if (UNLIKELY(!macro_is_i32(index_val))) {
        runtime_error(self, "[line %d] Expected integer index for 'op_vector_get', Found %s.",
          get_rle_line(&frame->closure->fn->chunk.lines, current_code_index(frame)),
          macro_type_name(index_val)
        );
        return interpret_runtime_error;
    }

    Instance* vec_instance = macro_as_instance(vec_val);
    Vec* vec = macro_as_vec(hashmap_get(
            &vec_instance->fields, new_string(self, "_data", 5)));
    int32_t index = macro_as_i32(index_val);

    if (UNLIKELY(index < 0 || index >= (int32_t)vec_len(vec))) {
        runtime_error(self, "[line %d] Vector index out of bounds (size=%d, index=%d).",
          get_rle_line(&frame->closure->fn->chunk.lines, current_code_index(frame)),
          vec_len(vec), index
        );
        return interpret_runtime_error;
    }

// 使用缓存预取
#ifdef __SSE2__
    _mm_prefetch((const char*)&vec->start[index], _MM_HINT_T0);
#endif
    MACRO_SAFE_PUSH(vec->start[index]);
    return interpret_ok;
}



static const OpMetadata __attribute__((unused)) op_meta[256] = {
        [op_pop]        = { "OP_POP", 0, handle_op_pop},
        [op_dup]        = { "OP_DUP", 0, handle_op_dup},
        [op_constant]   = { "OP_CONSTANT", 1, handle_op_constant},
        [op_constant_long] = { "OP_CONSTANT_LONG", 2, handle_op_constant_long},
        [op_value]      = { "OP_VALUE", 0, handle_op_value},
        [op_none]       = { "OP_NONE", 0, handle_op_none},
        [op_true]       = { "OP_TRUE", 0, handle_op_true},
        [op_false]      = { "OP_FALSE", 0, handle_op_false},
        [op_not]        = { "OP_NOT", 0, handle_op_not},
        [op_negate]     = { "OP_NEGATE", 0, handle_op_negate},
        [op_equal]      = { "OP_EQUAL", 0, handle_op_equal},
        [op_not_equal]  = { "OP_NOT_EQUAL", 0, handle_op_not_equal},
        [op_less]       = { "OP_LESS", 0, handle_op_less},
        [op_less_equal] = { "OP_LESS_EQUAL", 0, handle_op_less_equal},
        [op_greater]    = { "OP_GREATER", 0, handle_op_greater},
        [op_greater_equal] = { "OP_GREATER_EQUAL", 0, handle_op_greater_equal},
        [op_add]        = { "OP_ADD", 0, handle_op_add},
        [op_subtract]   = { "OP_SUBTRACT", 0, handle_op_subtract},
        [op_multiply]   = { "OP_MULTIPLY", 0, handle_op_multiply},
        [op_divide]     = { "OP_DIVIDE", 0, handle_op_divide},
        [op_bw_and]     = { "OP_BW_AND", 0, handle_op_bw_and},
        [op_bw_or]      = { "OP_BW_OR", 0, handle_op_bw_or},
        [op_bw_xor]     = { "OP_BW_XOR", 0, handle_op_bw_xor},
        [op_bw_sl]      = { "OP_BW_SL", 0, handle_op_bw_sl},
        [op_bw_sr]      = { "OP_BW_SR", 0, handle_op_bw_sr},
        [op_bw_not]     = { "OP_BW_NOT", 0, handle_op_bw_not},
        [op_define_global] = { "OP_DEFINE_GLOBAL", 1, handle_op_define_global},
        [op_get_global] = { "OP_GET_GLOBAL", 1, handle_op_get_global},
        [op_set_global] = { "OP_SET_GLOBAL", 1, handle_op_set_global},
        [op_get_local]  = { "OP_GET_LOCAL", 1, handle_op_get_local},
        [op_set_local]  = { "OP_SET_LOCAL", 1, handle_op_set_local},
        [op_get_upvalue]  = { "OP_GET_UPVALUE", 1, handle_op_get_upvalue},
        [op_set_upvalue]  = { "OP_SET_UPVALUE", 1, handle_op_set_upvalue},
        [op_get_property] = { "OP_GET_PROPERTY", 1, handle_op_get_property},
        [op_set_property] = { "OP_SET_PROPERTY", 1, handle_op_set_property},
        [op_get_super]  = { "OP_GET_SUPER", 1, handle_op_get_super},
        [op_get_layer_property] = { "OP_GET_LAYER_PROPERTY", 1, handle_op_get_layer_property},
        [op_get_type]   = { "OP_GET_TYPE", 1, handle_op_get_type},
        [op_jump_if_false] = { "OP_JUMP_IF_FALSE", 2, handle_op_jump_if_false},
        [op_jump_if_neq] = { "OP_JUMP_IF_NEQ", 2, handle_op_jump_if_neq},
        [op_jump]       = { "OP_JUMP", 2, handle_op_jump},
        [op_loop]       = { "OP_LOOP", 2, handle_op_loop},
        [op_call]       = { "OP_CALL", 1, handle_op_call},
        [op_closure]    = { "OP_CLOSURE", 2, handle_op_closure},
        [op_close_upvalue] = { "OP_CLOSE_UPVALUE", 0, handle_op_close_upvalue},
        [op_print]      = { "OP_PRINT", 0, handle_op_print},
        [op_return]     = { "OP_RETURN", 0, handle_op_return},
        [op_break]      = { "OP_BREAK", 0, handle_op_break},
        [op_continue]   = { "OP_CONTINUE", 0, handle_op_continue},
        [op_match]      = { "OP_MATCH", 0, handle_op_match},
        [op_class]      = { "OP_CLASS", 1, handle_op_class},
        [op_method]     = { "OP_METHOD", 1, handle_op_method},
        [op_inherit]    = { "OP_INHERIT", 0, handle_op_inherit},
        [op_invoke]     = { "OP_INVOKE", 2, handle_op_invoke},
        [op_super_invoke] = { "OP_SUPER_INVOKE", 2, handle_op_super_invoke},
        [op_struct]     = { "OP_STRUCT", 1, handle_op_struct},
        [op_member]     = { "OP_MEMBER", 1, handle_op_member},
        [op_struct_inherit] = { "OP_STRUCT_INHERIT", 0, handle_op_struct_inherit},
        [op_enum]       = { "OP_ENUM", 1, handle_op_enum},
        [op_enum_define_member] = { "OP_ENUM_DEFINE_MEMBER", 1, handle_op_enum_define_member},
        [op_enum_get_member]    = { "OP_ENUM_GET_MEMBER", 1, handle_op_enum_get_member},
        [op_enum_member_bind]   = { "OP_ENUM_MEMBER_BIND", 1, handle_op_enum_member_bind},
        [op_enum_member_match]  = { "OP_ENUM_MEMBER_MATCH", 1, handle_op_enum_member_match},
        [op_layer_property_call] = { "OP_LAYER_PROPERTY_CALL", 1, handle_op_layer_property_call},
        [op_vector_new] = { "OP_VECTOR_NEW", 1, handle_op_vector_new},
        [op_vector_set] = { "OP_VECTOR_SET", 1, handle_op_vector_set},
        [op_vector_get] = { "OP_VECTOR_GET", 1, handle_op_vector_get},
};


#undef macro_read_byte
#undef macro_read_byte_long
#undef macro_read_constant
#undef macro_read_constant_long
#undef macro_runtime_error_raised
#undef macro_read_string
#undef macro_read_short

/* just-in-time compilation(JIT) */
