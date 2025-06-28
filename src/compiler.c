//
// Created by Kilig on 2024/11/21.
//

/*
* “open area of research” and a “dark art.”
*
*  compiler:
*	worker 1: scanner user source code	(source code -> tokens)
* 	worker 2: parser source code to bytecode	(tokens -> bytecode)
*
*  Single-pass compilers
*		don’t need much surrounding context to understand a piece of syntax.
*
*  Two-pass compilers:
*		need to understand the entire program before generating bytecode.
*
*  function_pointer_array:
*		a common pattern in C to store function pointers in an array.
*		first column: prev_suffix_parse_function_pointer_array
*		second cloumn: middle_parse_function_pointer_array
*		third column: next_suffix_parse_function_pointer_array
*
*  left-associative operators(左结合运算符):
* 		a + b + c => (a + b) + c
*		对于左结合运算符，编译器会先编译左边的表达式，然后再编译右边的表达式。
*       对右操作数使用高一级的优先级，这样可以确保左操作数已经在栈顶。
*
*  right-associative operators(右结合运算符):
* 		a = b = c => a = (b = c)
*		对于右结合运算符，编译器会先编译右边的表达式，然后再编译左边的表达式。
*       使用与当前操作符相同的优先级，这样可以确保右操作数已经在栈顶。
*
*  Pratt解析器：
*		Pratt解析器是一种自顶向下的解析器，它使用了一种称为 “自上而下的预测”的技术。
*		它通过分析表达式的语法结构来确定操作符的优先级，并根据优先级来决定如何组合表达式。
*		Pratt解析器的工作原理是：
*			首先，它扫描表达式的输入，并生成一系列的标记。
*			然后，它使用一系列的规则来解析标记，这些规则描述了如何组合标记以生成表达式。
*			最后，它生成字节码，该字节码可以执行表达式的计算。
*		Pratt解析器的优点是：
*			它可以处理任意的表达式，而不管它们的语法结构如何。
*			它可以处理任意的左结合和右结合运算符。
*			它可以处理任意的运算符，而不管它们的优先级如何。
*		Pratt解析器的缺点是：
*			它需要编写大量的规则，以便它可以正确地解析表达式。
*			它需要维护一个栈，以便它可以正确地组合表达式。
*			它需要处理错误，以便它可以报告语法和语义错误。
*
*
*/

#include "common.h"
#include "string_.h"
#include "compiler.h"
#include "scanner.h"
#include "parser.h"

/*
* TODO: implement
* Then the runtime error will be reported on the wrong line.
* Here, it would show the error on line 2, even though the - is on line 1.
* A more robust approach would be to store the token’s line
* before compiling the operand and then pass that into emitByte(),
*/

static void __attribute__((unused)) free_local_variables(VirtualMachine *vm, LocalVariable local[], int count) {
	for (int i = 0; i < count; i++) {
		free_token(vm, &local[i].name);
	}
}

void begin_loop(Compiler* self, Loop *curr_loop, int start) {
    curr_loop->start = start;
    curr_loop->end = -1;
    curr_loop->enclosing = self->loop;
    self->loop = curr_loop;
}

void end_loop(Compiler* self){
    if(self->loop == NULL) {
        panic("{PANIC} [Compiler::end_loop] loop is NULL");
    }
    self->loop = self->loop->enclosing;
}


void init_compiler(Compiler* self, VirtualMachine *vm, FnType type) {
    self->vm = vm;
	self->enclosing = NULL;
	self->fn = NULL;
	self->local_count = 0;
	self->scope_depth = 0;
	self->fn_type = type;
	self->fn = new_fn(vm);	 // top-level function for example main()

	LocalVariable* local = &self->locals[self->local_count++];
	local->depth = 0;
	local->is_captured = false;
    switch (type) {
    case type_initializer:
    case type_method:
        local->name.type = token_self;
        local->name.start = "self";
        local->name.length = 4;
        local->name.line = 0;
        local->name.next = NULL;
        break;
    case type_fn:
    case type_lambda:
    case type_script:
        local->name.type = token_identifier;
        local->name.start = "";
        local->name.length = 0;
        local->name.line = 0;
        local->name.next = NULL;
        break;
    }

    self->loop = NULL;
}

void free_compiler(Compiler* self) {
    if (self != NULL) {
        // TODO: don't need to free fn, because they is object, vm will free it
        // free_fn(self->fn);
        // TODO:don't need to free local variables, because they ref to the tokens
        // free_local_variables(self->vm, self->locals, self->local_count);
        // reset compiler
        self->local_count = 0;
        self->scope_depth = 0;
        self->fn_type = type_script;
        self->fn = NULL;
        self->vm = NULL;
        self->loop = NULL;
    }
}

Chunk* curr_chunk(Compiler* self) {
	return &self->fn->chunk;
}

void compile_begin_scope(Compiler* self) {
	self->scope_depth++;
}
void compile_end_scope(Compiler* self) {
	self->scope_depth--;
}

void print_locals(Compiler* self) {
    for (int i = 0; i < self->local_count; i++) {
		printf("local[%d] ", i);
		print_token(&self->locals[i].name);
		printf(" depth: %d\n", self->locals[i].depth);
        printf("\n");
	}
}


Fn* compile(VirtualMachine* vm, const char* source) {
	Scanner scanner;
	init_scanner(&scanner, vm, source);
    scan_tokens(&scanner);

    // print_tokens(scanner.tokens);

	Parser parser;
	init_parser(&parser, vm, scanner.tokens);

	Compiler top_compiler;
	init_compiler(&top_compiler, vm, type_script);
	vm->compiler = &top_compiler;

	Fn* fn = parse_tokens(&parser, vm);

    // extend tokens: will tokens lifetime equal vm.
    list_extend_token(vm->tokens, scanner.tokens, vm);

    free_parser(&parser);
	free_scanner(&scanner);
    // free_compiler(vm->compiler) ; in parse_tokens() free

	// TODO: return compile status can be improved(e.g. return detailed error message and more status information)
	return parser.had_error ? NULL : fn;
}
