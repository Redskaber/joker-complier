//
// Created by Kilig on 2024/11/21.
//
/*
* Abstract Syntax Tree(AST):
*	An abstract syntax tree (AST) is a tree-like representation of the source code.
*	It is a way to represent the code in a way that is easy to manipulate and process.
*
* Context-Free Grammar:
*
* program        → declaration* EOF
*
* declaration    → class_decl | struct_decl | enum_decl | fn_decl | var_decl
*
* class_decl     → "class" IDENTIFIER (":" IDENTIFIER)? "{"
*                   (class_decl | struct_decl | fn_decl | var_decl)*
*                  "}"
*
* struct_decl    → "struct" IDENTIFIER (":" IDENTIFIER)? "{"
*                   (member ("," member)* ","?)?
*                  "}"
* member         → IDENTIFIER ":" type
*
* enum_decl      → "enum" IDENTIFIER "{"
*                   (IDENTIFIER ("(" type ("," type)* ")")?
*                   ("," IDENTIFIER ("(" type ("," type)* ")")? )*
*                 "}"
*
* fn_decl        → "fn" IDENTIFIER "(" (params)? ")" ("->" type)? blockStmt
* params         → IDENTIFIER ":" type ("," IDENTIFIER ":" type)*
*
* var_decl       → "var" IDENTIFIER (":" type)? ("=" expression)?
*                       ("," IDENTIFIER (":" type)? ("=" expression)?)? ";"
*
* statement      → exprStmt | returnStmt | breakStmt | continueStmt
*                 | matchStmt | ifStmt | printStmt | whileStmt | forStmt
*                 | loopStmt | blockStmt
*
* exprStmt       → expression ";"
* printStmt      → "print" expression ";"
* blockStmt      → "{" statement* "}"
* ifStmt         → "if" "(" expression ")" statement ("else" statement)?
* matchStmt      → "match" "(" expression ")" "{"
*                   (pattern "=>" statement ",")*
*                   (pattern "=>" statement)
*                 "}"
* pattern        → literal | IDENTIFIER | "_"
*
* expression     → ternary
* ternary        → logic_or ("?" expression ":" ternary)?
* logic_or       → logic_and ("or" logic_and)*
* logic_and      → equality ("and" equality)*
* equality       → comparison (("==" | "!=") comparison)*
* comparison     → term ((">" | ">=" | "<" | "<=") term)*
* term           → factor (("+" | "-") factor)*
* factor         → unary (("*" | "/") unary)*
* unary          → ("!" | "-") unary | call
* call           → primary ("(" arguments? ")" | "." IDENTIFIER)*
* arguments      → expression ("," expression)*
* primary        → literal | IDENTIFIER | "(" expression ")" | lambda
* lambda         → "|" (params)? "|" ("->" type)? blockStmt
*
* literal        → NUMBER | STRING | "true" | "false" | "None"
* type           → IDENTIFIER
*                  | "i8" | "u8"
*                  | "i16" | "u16"
*                  | "i32" | "u32" | "f32"
*                  | "i64" | "f64"
*                  | "usize" | "isize"
*                  | "string"
*
*
*
* stack effect
*
* TODO: handler codes: uint8_t && index_t relations;
* TODO: global variables store and find optimization.
*
* TODO: static check: e.g.
*   ```joker
*       fn use_var() {
*           print oops; // error: variable 'oops' is not defined.
*       }
*
*      var oops = "too many o's";
*   ```
*   should in compile time error.?
*
* TODO: add keyword 'const' to declare constant variable.
* TODO: could have separate instructions for conditional jumps that implicitly pop and those that don’t.
* TODO: add match keyword to match pattern in switch statement.
* TODO: add ?: operator to handle ternary operator.
* TODO：add 'continue' and 'break' statement to loop statement.
* TODO: 'goto'?
* TODO: jump table used ?
* TODO: lambda function? anonymous function?
* TODO: closure optimization? used bit operation? uint8_t [1bit{is_local}, 7bit{index}]? optimize?
*     - closure: value is 'value' or 'variable'? [variable]
*     - upvalue 1.closed heap pos? 2.when closed?
*         - 1. Closure->upvalue_ptrs {base, local} => {base, local, closed}
*         - 2. stack time long to long good, in scope in stack, closed scope call variable raise error. (so, compiler need nonworking variable is? closed)
*     - Tracking open upvalues(runtime question):
*         - 1. more closure visit once variable? => store once pos, a(write 10) -> b(read 10)
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "common.h"
#include "fn.h"
#include "error.h"
#include "string_.h"
#include "compiler.h"
#include "class_compiler.h"
#include "parser.h"

#if debug_print_code
#include "debug.h"
#endif


/* Parser */
static void synchronize(Parser* self);
static Token* parse_advance(Parser* self);
static bool parse_check(Parser* self, TokenType type);
static bool parse_match(Parser* self, TokenType type);
static bool parse_is_at_end(Parser* self);
static void parse_consume(Parser* self, TokenType type, const char* message);
static void parse_error_at(Parser* self, Token* error, const char* message);
void parse_error_at_curr(Parser* self, const char* message);
void parse_error_at_prev(Parser* self, const char* message);

/* Abstract Syntax Tree(AST)*/
static void parse_declaration(Parser* self, VirtualMachine* vm);
static void parse_enum_declaration(Parser* self, VirtualMachine* vm);
static void parse_enum_member(Parser* self, VirtualMachine* vm);
static void parse_struct_declaration(Parser* self, VirtualMachine* vm);
static void parse_member(Parser* self, VirtualMachine* vm);
static void parse_class_declaration(Parser* self, VirtualMachine* vm);
static void parse_class_member(Parser *self, VirtualMachine *vm);
static void parse_method(Parser *self, VirtualMachine *vm);
static uint8_t parse_argument_list(Parser* self, VirtualMachine* vm);
static void parse_fn_declaration(Parser* self, VirtualMachine* vm);
static void parse_function(Parser* self, VirtualMachine* vm, FnType type);
static void parse_var_declaration(Parser* self, VirtualMachine* vm);
static void parse_named_variable(Parser* self, VirtualMachine* vm, Token* var_name, bool can_assign);
static index_t parse_variable(Parser* self, VirtualMachine* vm, const char* message);
static void parse_statement(Parser* self, VirtualMachine* vm);
static void parse_if_statement(Parser* self, VirtualMachine* vm);
static void parse_while_statement(Parser* self, VirtualMachine* vm);
static void parse_for_statement(Parser* self, VirtualMachine* vm);
static void parse_loop_statement(Parser* self, VirtualMachine* vm);
static void parse_block_statement(Parser* self, VirtualMachine* vm);
static void parse_print_statement(Parser* self, VirtualMachine* vm);
static void parse_expr_statement(Parser* self, VirtualMachine* vm);
static void parse_break_statement(Parser* self, VirtualMachine* vm);
static void parse_match_statement(Parser* self, VirtualMachine* vm);
static void parse_match_member(Parser* self, VirtualMachine* vm, Match *match);
static void parse_continue_statement(Parser* self, VirtualMachine* vm);
static void parse_return_statement(Parser* self, VirtualMachine* vm);
static void parse_precedence(Parser* self, VirtualMachine* vm, Precedence outer_precedence);
static void parse_expression(Parser* self, VirtualMachine* vm);


/* Compiler */
static index_t make_constant(Parser* self, Chunk* chunk, Value value);
static index_t identifier_constant(Parser* self, VirtualMachine* vm, Token* token);
static void emit_constant(Parser* self, Chunk* chunk, Value value);
static void emit_return(Parser* self, Chunk* chunk);
static void emit_byte(Parser* self, Chunk* chunk, uint8_t byte);
static void emit_bytes(Parser* self, Chunk* chunk, uint8_t opcode, uint8_t operand);
static int emit_jump(Parser* self, Chunk* chunk, uint8_t opcode);
static void patch_jump(Parser* self, Chunk* chunk, int offset);
static void emit_loop(Parser* self, Chunk* chunk, int loop_start);
static void emit_continue(Parser* self, Chunk* chunk, int loop_start);
static void emit_break(Parser* self, Chunk* chunk);

static void begin_scope(Compiler* compiler);
static void end_scope(Parser* self, Compiler* compiler, Chunk* chunk);
static void curr_into_sub_compiler(VirtualMachine* vm, Compiler* sub, FnType type, Parser* parser);
static Fn* curr_from_sub_compiler(Parser* self, VirtualMachine* vm);
static bool identifiers_equal(Token* left, Token* right);
static void declare_variable(Parser* self, Compiler* compiler, Token* name);
static void define_variable(Parser* self, Compiler* compiler, uint8_t index);
static void add_local(Parser* self, Compiler* compiler, Token* name);
static void mark_initialized(Compiler* compiler);
static int resolve_local(Parser* self, Compiler* compiler, Token* name);


/* get handle token's rule(prefix, infix, ...)[function pointer] */
ParseRule* get_rule(TokenType type) {
	return &static_syntax_rules[type];
}

void init_parser(Parser* self, VirtualMachine *vm, TokenList* tokens) {
    self->vm = vm;
	self->tokens = tokens;
	self->curr = tokens->head;
	self->prev = NULL;
    self->pprev = NULL;
	self->had_error = false;
	self->panic_mode = false;
}

void free_parser(Parser* self) {
    self->vm = NULL;
	self->tokens = NULL;
	self->curr = NULL;
	self->prev = NULL;
    self->pprev = NULL;
	self->had_error = false;
	self->panic_mode = false;
}

void __attribute__((unused)) print_parser(Parser* self) {
	printf("Parser:\n");
	printf("tokens: "); print_tokens(self->tokens);
	printf("curr: "); print_token(self->curr); printf("\n");
	printf("prev: "); print_token(self->prev); printf("\n");
    printf("pprev: "); print_token(self->pprev);
	printf("had_error: %d\n", self->had_error);
	printf("panic_mode: %d\n", self->panic_mode);
}

/* check if is at end of file, return true if is at end of file, false otherwise */
static bool parse_is_at_end(Parser* self) {
	if (self->curr == NULL) {
		parse_error_at_prev(self, "[Parser::parse_is_at_end] Expected match end of file(eof token), Found Unexpected end of file(not a token).");
		return true;
	}
	return self->curr->type == token_eof;
}

/* advance to next token, return prev token, if is at end of file, return NULL */
static Token* parse_advance(Parser* self) {
	if (parse_is_at_end(self)) return NULL;
    self->pprev = self->prev;
	self->prev = self->curr;
	self->curr = self->curr->next;
	return self->prev;
}

static bool parse_check(Parser* self, TokenType type) {
	if (self->curr == NULL) return false;
	return self->curr->type == type;
}

static bool parse_match(Parser* self, TokenType type) {
	if (!parse_check(self, type)) return false;
	parse_advance(self);
	return true;
}

static void parse_consume(Parser* self, TokenType type, const char* message) {
	if (self->curr->type == type) {
		parse_advance(self);
		return;
	}
	parse_error_at_curr(self, message);
}

static void emit_byte(Parser* self, Chunk* chunk, uint8_t byte) {
	write_chunk(chunk, byte, self->prev->line);
}

static void emit_constant(Parser* self, Chunk* chunk, Value value) {
	write_constant(chunk, value, self->prev->line);
}

static void emit_bytes(Parser* self, Chunk* chunk, uint8_t opcode, uint8_t operand) {
	emit_byte(self, chunk, opcode);
	emit_byte(self, chunk, operand);
}

static void emit_return(Parser* self, Chunk* chunk) {
    Compiler *curr_compiler = self->vm->compiler;
    if (curr_compiler == NULL) {
        panic("[Parser::emit_return] No current compiler.");
    }

    // type initialized to 0
    if (curr_compiler->fn_type == type_initializer) {
        emit_bytes(self, chunk, op_get_local, 0);
    }

	emit_byte(self, chunk, op_return);
}

/*
* chunk->count - 2: return index of jump offset
*						|----------< -2 >---------|
*						V						  V
* [code, code , {code, jump_offset, jump_offset}, ...]
*/
static int emit_jump(Parser* self, Chunk* chunk, uint8_t opcode) {
	emit_byte(self, chunk, opcode);
	emit_byte(self, chunk, 0xff);		// placeholder for jump offset
	emit_byte(self, chunk, 0xff);		// placeholder for jump offset 2^(8+8) = 65535
	return chunk->count - 2;			// return index of jump offset
}
/*
* patch_jump:
*	- calculate jump offset: chunk->count - offset - 2
*	- write high 8 bits of jump offset to chunk->code[offset]
* 	- write low 8 bits of jump offset to chunk->code[offset+1]
*
*   | --< offset >-- |									  |(chunk->count)
*	V				 V |<---------(2)--------->|<-jump->| V
* [code, code , {jump, jump_offset, jump_offset}, ......, code,code, ...]
*/
static void patch_jump(Parser* self, Chunk* chunk, int offset) {
	int jump = chunk->count - offset - 2;
	if (jump > UINT16_MAX) {
		parse_error_at_curr(self, "[Parser::patch_jump] Expected jump offset less than 2^16, Found jump offset greater than 2^16.");
	}

	chunk->code[offset] = (jump >> 8) & 0xff;   // high 8 bits of jump offset
	chunk->code[offset + 1] = jump & 0xff;		// low 8 bits of jump offset
}

/*
* emit_loop:
*	- emit op_loop
*	- calculate loop offset: chunk->count - loop_start + 2
* 	- emit high 8 bits of loop offset
* 	- emit low 8 bits of loop offset
*
*	while
*    |     <- loop_start  <-——--- loop_start
*    |     (				↑
*    |     condition		|
*    |     )				|
*    |     <- exit_jump		|
*    |     {				|
*    |         statement	|
*    |     }				|
*    |     <--- op_loop --->|
*    |        | loop_offset |
*    |        V loop_offset | <- chunck->count
*	=> offset = chunk->count - loop_start + 2(loop_offset)
*/
static void emit_loop(Parser* self, Chunk* chunk, int loop_start) {
	emit_byte(self, chunk, op_loop);

	int offset = chunk->count - loop_start + 2;
	if (offset > UINT16_MAX) {
		parse_error_at_curr(self, "[Parser::emit_loop] Expected loop offset less than 2^16, Found loop offset greater than 2^16.");
	}

	emit_byte(self, chunk, (offset >> 8) & 0xff);	// high 8 bits of loop offset
	emit_byte(self, chunk, offset & 0xff);			// low 8 bits of loop offset
}

static void emit_continue(Parser* self, Chunk* chunk, int loop_start) {
    emit_byte(self, chunk, op_continue);

    int offset = chunk->count - loop_start + 2;
    if (offset > UINT16_MAX) {
        parse_error_at_curr(self, "[Parser::emit_loop] Expected loop offset less than 2^16, Found loop offset greater than 2^16.");
    }

    emit_byte(self, chunk, (offset >> 8) & 0xff);	// high 8 bits of loop offset
    emit_byte(self, chunk, offset & 0xff);			// low 8 bits of loop offset
}

static void emit_break(Parser* self, Chunk* chunk) {
    Loop* curr_loop = self->vm->compiler->loop;
    if (curr_loop == NULL) {
        parse_error_at_curr(self, "[Parser::emit_break] Can't use 'break' outside of a loop.");
        return;
    }
    patch_jump(self, chunk, curr_loop->end);
}

static index_t make_constant(Parser* self, Chunk* chunk, Value value) {
	index_t index = add_constant(chunk, value);
	if (index > UINT16_MAX) {
		parse_error_at_curr(self, "[Parser::make_constant] Expected constant index less than 2^16, Found constant index greater than 2^16.");
	}
	return index;
}

static bool identifiers_equal(Token* left, Token* right) {
    return equal_token(left, right);
}

static Token synthetic_token(const char* text) {
    return make_token(
            token_identifier,
            text,
            (int)strlen(text),
            0
            );
}

/*  “Declaring” is when the variable is added to the scope, and “defining” is when it becomes available for use */
static void declare_variable(Parser* self, Compiler* compiler, Token* name) {
	if (compiler->scope_depth == 0) return;

	for (int i = compiler->local_count - 1; i >= 0; i--) {
		LocalVariable* local = &compiler->locals[i];
		if (local->depth != -1 && local->depth < compiler->scope_depth) {
			break;
		}

		if (identifiers_equal(&local->name, name)) {
			parse_error_at_curr(self, "[Parser::declare_variable] Variable with this name already declared in this scope.");
		}
	}

	add_local(self, compiler, name);
}

static void define_variable(Parser* self, Compiler* compiler, uint8_t index) {
	if (compiler->scope_depth > 0) {
		mark_initialized(compiler);
		return;
	}
	emit_bytes(self, curr_chunk(compiler), op_define_global, index);
}

/* identifier is string constant so big, so we add to constants table, through index get value
* TODO: string -> constant table { (more reference same value)↓ -> memory optimization }
*/
static index_t identifier_constant(Parser* self, VirtualMachine* vm, Token* token) {
	return make_constant(self, curr_chunk(vm->compiler), macro_val_from_obj(new_string(vm, token->start, token->length)));
}

static void add_local(Parser* self, Compiler* compiler, Token* name) {
	if (compiler->local_count == uint8_count) {
		parse_error_at_curr(self, "[Parser::add_local] Expected local variable count less than 256, Found local variable count greater than 256.");
		return;
	}
	LocalVariable* local = &compiler->locals[compiler->local_count++];
	local->name = make_token(
        name->type,
        name->start,
        name->length,
        name->line
    );
	local->depth = -1;			// default depth is -1, means not defined.
	local->is_captured = false;
}

static void mark_initialized(Compiler* compiler) {
	if (compiler->scope_depth == 0) return; // global scope, no need to mark initialized.
	compiler->locals[compiler->local_count - 1].depth = compiler->scope_depth;
}

// TODO: optimize: use hashmap to store local variables.[hashmap? or array? or linked list? or other?]
static int resolve_local(Parser* self, Compiler* compiler, Token* name) {
	for (int i = compiler->local_count - 1; i >= 0; i--) {
		LocalVariable* local = &compiler->locals[i];
		if (identifiers_equal(&local->name, name)) {
			if (local->depth == -1) {
				parse_error_at_curr(self, "[Parser::resolve_local] Local variable with this name is not initialized in this scope.");
			}
			return i;	// found: return index
		}
	}
	return -1;	// not found
}

/* add upvalue to function, return index of upvalue;
* upvalue range: 0~127, so we can only use 127 upvalue parameters.
*/
static int add_upvalue(Parser* self, Compiler* compiler, upvalue_info_t upvalue_info) {
	int upvalue_count = compiler->fn->upvalue_count;
	// this index count is 7bits, so we can only use 127 upvalue parameters.
	if (upvalue_count == upvalue_index_count) {
		parse_error_at_curr(self, "[Parser::add_upvalue] Expected upvalue count less than 128, Found upvalue count greater than 128.");
		return 0;
	}
	for (int i = 0; i < upvalue_count; i++) {
		if (compiler->upvalues[i] == upvalue_info) {
			return i;
		}
	}
	compiler->upvalues[upvalue_count] = upvalue_info;
	return compiler->fn->upvalue_count++;
}

/* {,....val,...,Fn...}
*									|					(-1)
* Fn outer() {					 <- | (find up value)	(local index)
*     Fn middle() {				 <- | (find up value)	(local index)
*         Fn inner() {				|
*            ...               ---> |
*         }
*     }
* }
*
* outer(){local} -> middle(){local} -> inner(){local}	 局部变量内部穿透
* TODO: recursive up local function local variable? => optimize?
*/
static int resolve_upvalue(Parser* self, Compiler* compiler, Token* name) {
	if (compiler->enclosing == NULL) return -1;
	if (compiler->fn_type == type_script) return -1;

	int up_local = resolve_local(self, compiler->enclosing, name);
	if (up_local != -1) {
		// upvalue found in enclosing scope, mark as upvalue.
		compiler->enclosing->locals[up_local].is_captured = true;
		upvalue_info_t upvalue_info = new_upvalue_info((uint8_t)up_local, true);
		return add_upvalue(self, compiler, upvalue_info);
	}

	int up_upvalue = resolve_upvalue(self, compiler->enclosing, name);
	if (up_upvalue != -1) {
		// upvalue found in enclosing scope, mark as upvalue.
		upvalue_info_t upvalue_info = new_upvalue_info((uint8_t)up_upvalue, false);
		return add_upvalue(self, compiler, upvalue_info);
	}

	return -1;
}

static void begin_scope(Compiler* compiler) {
	compile_begin_scope(compiler);
}

static void end_scope(Parser* self, Compiler* compiler, Chunk* chunk) {
	compile_end_scope(compiler);

	// free local variables
	while (compiler->local_count > 0 &&
		compiler->locals[compiler->local_count - 1].depth > compiler->scope_depth) {
		if (compiler->locals[compiler->local_count - 1].is_captured) {
			emit_byte(self, chunk, op_close_upvalue);	// close upvalue; not need to free local variable.
		}
		else {
			emit_byte(self, chunk, op_pop);	// TODO: op_popn? optimize?
		}
		compiler->local_count--;
	}
}

static void curr_into_sub_compiler(VirtualMachine* vm, Compiler* sub, FnType type, Parser* parser) {
	// set sub compiler
	init_compiler(sub, vm, type);
	sub->enclosing = vm->compiler;
	// set vm compiler to sub compiler
	vm->compiler = sub;

	if (type != type_script) {
		sub->fn->name = new_string(vm, parser->prev->start, parser->prev->length);
	}
}

static Fn* curr_from_sub_compiler(Parser* self, VirtualMachine* vm) {
    emit_return(self, curr_chunk(vm->compiler));

    // build func return && return parent compiler's func
    Fn* fn = vm->compiler->fn;
#if debug_print_code
    if (!self->had_error) {
        disassemble_chunk(curr_chunk(vm->compiler),
                          is_anonymous_fn(fn) ? "<script>" : fn->name->chars);
    }
#endif
    /*
        vm -> main->compiler					--- ↑  => NULL
        |		|								|
        |		|   sub_compiler            --- ↑
        |			|						|
        |			|   sub_compiler	--- ↑

    */
    // sub compiler isn't free, so need to free it
    vm->compiler = vm->compiler->enclosing;
    return fn;
}

static void synchronize(Parser* self) {
	self->panic_mode = false;

	while (!parse_is_at_end(self)) {
		if (self->prev->type == token_semicolon) return;

		switch (self->curr->type)
		{
		case token_class:
		case token_fn:
		case token_var:
		case token_for:
		case token_if:
		case token_while:
#ifndef deprecated_print_keyword
		case token_print:
#endif
		case token_return: return;
		default: break;
		}
		parse_advance(self);
	}
}

Fn* parse_tokens(Parser* self, VirtualMachine* vm) {
	while (!parse_is_at_end(self)) {
		parse_declaration(self, vm);
	}
	parse_consume(self, token_eof, "Expected end of expression.");
	return curr_from_sub_compiler(self, vm);
}

static void parse_declaration(Parser* self, VirtualMachine* vm) {
	if (parse_match(self, token_class)) {
        parse_class_declaration(self, vm);
    } else if (parse_match(self, token_struct)) {
        parse_struct_declaration(self, vm);
    } else if (parse_match(self, token_enum)) {
        parse_enum_declaration(self, vm);
    } else if (parse_match(self, token_fn)) {
		parse_fn_declaration(self, vm);
	} else if (parse_match(self, token_var)) {
		parse_var_declaration(self, vm);
	} else {
		parse_statement(self, vm);
	}
	if (self->panic_mode) synchronize(self);
}

/*
 * enum Declaration: enum identifier "{" member* "}"
 */
static void parse_enum_declaration(Parser* self, VirtualMachine* vm) {
    parse_consume(self, token_identifier, "[Parser::parse_enum_declaration] Expected enum name.");
    Token* enum_name = self->prev;
    index_t name_index = identifier_constant(self, vm, self->prev);
    declare_variable(self, vm->compiler, self->prev);

    emit_bytes(self, curr_chunk(vm->compiler), op_enum, name_index);
    define_variable(self, vm->compiler, name_index);
    parse_named_variable(self, vm, enum_name, false);

    parse_consume(self, token_left_brace, "[Parser::parse_enum_declaration] Expected '{' after enum name.");
    while (!parse_check(self, token_right_brace)) {
        parse_enum_member(self, vm);
    }

    parse_consume(self, token_right_brace, "[Parser::parse_enum_declaration] Expected '}' after enum members.");
    emit_byte(self, curr_chunk(vm->compiler), op_pop);
}

static void parse_enum_member(Parser* self, VirtualMachine* vm) {
    parse_consume(self, token_identifier, "[Parser::parse_enum_member] Expected member name.");
    index_t name_index = identifier_constant(self, vm, self->prev);

    int arg_count = 0;
    if (parse_match(self, token_left_paren)) {
        while(!parse_match(self, token_right_paren)) {
            // TODO: stored pattern type
            // TODO:
            switch (self->curr->type) {
                case token_identifier: {
                    parse_advance(self);
                    parse_named_variable(self, vm, self->prev, false);
                    break;
                }
                default: panic("[Parser::parse_enum_member] Expected member value.");
            }

            arg_count++;
            if (!parse_check(self, token_right_paren)) {
                parse_consume(self, token_comma, "[Parser::parse_enum_member] Expected ',' after member value.");
            }
        }
        if (arg_count < 1) {
            panic("[Parser::parse_enum_member] Expected at least one member value.");
        }
        emit_bytes(self, curr_chunk(vm->compiler), op_value, arg_count);
    } else {
        emit_bytes(self, curr_chunk(vm->compiler), op_value, 0);
    }

    emit_bytes(self, curr_chunk(vm->compiler), op_enum_define_member, name_index);
    if (!parse_check(self, token_right_brace)) {
        parse_consume(self, token_comma, "[Parser::parse_enum_member] Expected ',' after member declaration.");
    }
}

/* TODO: structDeclaration => to parse
 * structDeclaration: "struct" identifier (":" super_struct) "{" ((member ",")?  member)? "}"
 */
static void parse_struct_declaration(Parser* self, VirtualMachine* vm) {
    parse_consume(self, token_identifier, "[Parser::parse_struct_declaration] Expected struct name.");
    Token* struct_name = self->prev;
    index_t name_index = identifier_constant(self, vm, self->prev);
    declare_variable(self, vm->compiler, self->prev);

    emit_bytes(self, curr_chunk(vm->compiler), op_struct, name_index);
    define_variable(self, vm->compiler, name_index);

    // TODO: struct A: B {}
    if(parse_match(self, token_colon)) {
        parse_consume(self, token_identifier, "[Parser::parse_struct_declaration] Expected super struct name.");
        parse_named_variable(self, vm, self->prev, false);

        if (identifiers_equal(self->prev, struct_name)){
            parse_error_at_curr(self, "[Parser::parse_struct_declaration] A struct can't inherit from itself.");
        }

        parse_named_variable(self, vm, struct_name, false);
        emit_byte(self, curr_chunk(vm->compiler), op_struct_inherit);
    }
    parse_named_variable(self, vm, struct_name, false);

    parse_consume(self, token_left_brace, "[Parser::parse_struct_declaration] Expected '{' before struct body.");
    while(!parse_check(self, token_right_brace) && !parse_check(self, token_eof)) {
        parse_member(self, vm);
    }

    parse_consume(self, token_right_brace, "[Parser::parse_struct_declaration] Expected '}' after struct body.");
    emit_byte(self, curr_chunk(vm->compiler), op_pop);
}

static void parse_member(Parser* self, VirtualMachine* vm) {
    parse_consume(self, token_identifier, "[Parser::parse_method] Expected method name.");
    index_t name_index = identifier_constant(self, vm, self->prev);

    // TODO: type label
    if (parse_match(self, token_colon)) {
        // var declaration || var assign
        while(!parse_check(self, token_right_brace)
              && !parse_check(self, token_comma)) {
            parse_advance(self);
        }
    }
    emit_byte(self, curr_chunk(vm->compiler), op_none);	 // variable value default is null

    emit_bytes(self, curr_chunk(vm->compiler), op_member, name_index);
    if (!parse_check(self, token_right_brace)) {
        parse_consume(self, token_comma, "[Parser::parse_member] Expected ',' after member declaration.");
    }
}

static void parse_class_declaration(Parser* self, VirtualMachine* vm) {
    parse_consume(self, token_identifier, "[Parser::parse_class_declaration] Expected class name.");
    Token* class_name = self->prev;


    index_t name_index = identifier_constant(self, vm, self->prev);
    declare_variable(self, vm->compiler, self->prev);

    emit_bytes(self, curr_chunk(vm->compiler), op_class, name_index);
    define_variable(self, vm->compiler, name_index);

    // class compiler
    ClassCompiler class_compiler;
    into_curr_class_compiler(&class_compiler, vm);

    // class A: B {}
    if (parse_match(self, token_colon)) {
        parse_consume(self, token_identifier, "[Parser::parse_class_declaration] Expected superclass name.");
        parse_named_variable(self, vm, self->prev, false);

        // TODO: check superclass
        if (identifiers_equal(class_name, self->prev)) {
            parse_error_at_curr(self, "[Parser::parse_class_declaration] A class can't inherit from itself.");
        }

        // add superclass to scope (class compiler)
        compile_begin_scope(vm->compiler);
        Token super = synthetic_token("super");
        add_local(self, vm->compiler, &super);
        define_variable(self, vm->compiler, 0);

        parse_named_variable(self, vm, class_name, false);
        emit_byte(self, curr_chunk(vm->compiler), op_inherit);

        // set superclass
        set_curr_class_superclass(&class_compiler, true);
    }

    parse_named_variable(self, vm, class_name, false);
    parse_consume(self, token_left_brace, "[Parser::parse_class_declaration] Expected '{' before class body.");
    while(!parse_check(self, token_right_brace) && !parse_check(self, token_eof)) {
        parse_class_member(self, vm);
    }
    parse_consume(self, token_right_brace, "[Parser::parse_class_declaration] Expected '}' after class body.");
    emit_byte(self, curr_chunk(vm->compiler), op_pop);

    // end superclass scope(class compiler)
    if (class_compiler.has_superclass) {
        compile_end_scope(vm->compiler);
    }

    // out class compiler
    out_curr_class_compiler(vm);
}

/*
 * classMember: "fn" identifier "(" parameters? ")" block
 * classMember: "class" identifier ":" identifier "{" member* "}"
 * classMember: "struct" identifier ":" identifier "{" member* "}"
 * classMember: "var" identifier (":" type)? ("=" expression)?
 */
static void parse_class_member(Parser *self, VirtualMachine *vm) {
    if (parse_match(self, token_class)) {
        parse_class_declaration(self, vm);
    } else if (parse_match(self, token_struct)){
        parse_struct_declaration(self, vm);
    } else if (parse_match(self, token_fn)) {
        parse_method(self, vm);
    } else {
        parse_error_at_curr(self, "[Parser::parse_class_member] Expected method or member.");
    }
}

static void parse_method(Parser *self, VirtualMachine *vm) {
    parse_consume(self, token_identifier, "[Parser::parse_method] Expected method name.");
    index_t name_index = identifier_constant(self, vm, self->prev);

    // parse method for class
    FnType fy = type_method;
    if (self->prev->length == 4 &&
        memcmp(self->prev->start, "init", 4) == 0) {
        fy = type_initializer;
    }

    parse_function(self, vm, fy);
    emit_bytes(self, curr_chunk(vm->compiler), op_method, name_index);
}

void parse_self(Parser* self, VirtualMachine* vm, bool _can_assign) {
    (void)_can_assign;

    if (vm->class_compiler == NULL) {
        parse_error_at_curr(self, "[Parser::parse_self] Can't use 'self' outside of a class.");
        return;
    }
    parse_identifier(self, vm, false);
}

void parse_super(Parser* self, VirtualMachine* vm, bool _can_assign) {
    (void)_can_assign;

    if (vm->class_compiler == NULL) {
        parse_error_at_curr(self, "[Parser::parse_super] Can't use 'super' outside of a class.");
        return;
    } else if (!vm->class_compiler->has_superclass) {
        parse_error_at_curr(self, "[Parser::parse_super] Can't use 'super' in a class with no superclass.");
        return;
    }
    // parser
    parse_consume(self, token_dot, "[Parser::parse_super] Expected '.' after 'super'.");
    parse_consume(self, token_identifier, "[Parser::parse_super] Expected superclass method name.");
    index_t method_index = identifier_constant(self, vm, self->prev);

    // compiler
    Chunk* chunk = curr_chunk(vm->compiler);
    Token self_ = synthetic_token("self");
    parse_named_variable(self, vm, &self_, false);

    // check if it's a call or a get
    Token super = synthetic_token("super");
    if (parse_match(self, token_left_paren)) {
        uint8_t arg_count = parse_argument_list(self, vm);
        parse_named_variable(self, vm, &super, false);
        emit_bytes(self, chunk, op_super_invoke, method_index);
        emit_byte(self, chunk, arg_count);
    } else {
        parse_named_variable(self, vm, &super, false);
        emit_bytes(self, chunk, op_get_super, method_index);
    }
}

static void parse_fn_declaration(Parser* self, VirtualMachine* vm) {
	index_t func_index = parse_variable(self, vm, "[Parser::parse_fn_declaration] Expected function name.");
	mark_initialized(vm->compiler);

	parse_function(self, vm, type_fn);
	define_variable(self, vm->compiler, func_index);
}

static void parse_function(Parser* self, VirtualMachine* vm, FnType type) {
	Compiler func_compiler;
	curr_into_sub_compiler(vm, &func_compiler, type, self);
    // TODO: this compiler is a sub-compiler(func_compiler), so it has its own scope.
    begin_scope(vm->compiler);

    parse_consume(self, token_left_paren, "[Parser::parse_function] Expected '(' after function name.");
	// parameter list
	if (!parse_check(self, token_right_paren)) {
		do {
			vm->compiler->fn->arity++;
			if (vm->compiler->fn->arity > argument_count_max) {
				parse_error_at_curr(self, "[Parser::parse_function] Expected function parameter count less than 256, Found function parameter count greater than 256.");
				return; // TODO: error
			}

			uint8_t var_index = parse_variable(self, vm, "[Parser::parse_function] Expected parameter name.");
			define_variable(self, vm->compiler, var_index);

            // TODO: type label (parameter: type)*?
            if (parse_match(self, token_colon)) {
                do {
                    parse_advance(self);
                    if (parse_check(self, token_right_paren)) {
                        break;
                    }
                }while(!parse_check(self, token_comma));
            }

		} while (parse_match(self, token_comma));
	}
	parse_consume(self, token_right_paren, "[Parser::parse_function] Expected ')' after function parameters.");

    // TODO: type label (return type)*?
    if (parse_match(self, token_arrow)) {
        while(!parse_check(self, token_left_brace)) {
            parse_advance(self);
        }
    }

	parse_consume(self, token_left_brace, "[Parser::parse_function] Expected '{' before function body.");

	parse_block_statement(self, vm);

	// get func object && emit define instruction, func_compiler -> up_compiler
	Fn* fn = curr_from_sub_compiler(self, vm);

	// default all function include in closure.
    Chunk *curr_ck = curr_chunk(vm->compiler);
    index_t func_index = make_constant(self, curr_ck, macro_val_from_obj(fn));
	emit_bytes(self,curr_ck,op_closure,func_index);
	// push function up value to stack; used bit {1bit: is_local, 7bit: index}
	for (int i = 0; i < fn->upvalue_count; i++) {
		emit_byte(self,curr_ck,func_compiler.upvalues[i]);
	}

    // TODO: free func compiler; don't need to free it.(reset)
    free_compiler(&func_compiler);
}

/*
* parse variable declaration:
*	- parse_variable(identifier -> constant table && return index): parse variable name, add to constants table, return index.
*   - parse_expression(value -> constant table): parse expression after equal sign, if is not exist, emit null.
*   - define_variable(variable type -> constant table): emit define_global instruction to define variable.
*/
static void parse_var_declaration(Parser* self, VirtualMachine* vm) {
    do {
        // 1. 解析变量名
        uint8_t index = parse_variable(self, vm, "[Parser::parse_var_declaration] Expected variable name.");

        // 2. 处理类型标注（语法树暂不处理类型）
        if (parse_match(self, token_colon)) {
            while (!parse_check(self, token_assign) &&
                   !parse_check(self, token_comma) &&
                   !parse_check(self, token_semicolon)) {
                parse_advance(self); // 跳过类型标记
            }
        }

        // 3. 处理初始化表达式
        if (parse_match(self, token_assign)) {
            parse_expression(self, vm);
        } else {
            emit_byte(self, curr_chunk(vm->compiler), op_none);
        }

        // 4. 定义变量到当前作用域
        define_variable(self, vm->compiler, index);

    } while (parse_match(self, token_comma)); // 5. 支持逗号分隔的多变量声明

    // 6. 结束声明
    parse_consume(self, token_semicolon, "[Parser::parse_var_declaration] Expected ';' after variable declaration.");
}

static index_t parse_variable(Parser* self, VirtualMachine* vm, const char* message) {
	parse_consume(self, token_identifier, message);

	declare_variable(self, vm->compiler, self->prev);
	if (vm->compiler->scope_depth > 0) return 0;

	return identifier_constant(self, vm, self->prev);
}

void parse_identifier(Parser* self, VirtualMachine* vm, bool can_assign) {
	parse_named_variable(self, vm, self->prev, can_assign);
}

static void parse_named_variable(Parser* self, VirtualMachine* vm, Token* var_name, bool can_assign) {
	index_t index = resolve_local(self, vm->compiler, var_name);
	uint8_t get_op, set_op;

	/* global var | up var | local var */
	if (index != -1) {
		get_op = op_get_local;
		set_op = op_set_local;
	}
	else if ((index = resolve_upvalue(self, vm->compiler, var_name)) != -1) {
		get_op = op_get_upvalue;
		set_op = op_set_upvalue;
	}
	else {
		index = identifier_constant(self, vm, var_name);
		get_op = op_get_global;
		set_op = op_set_global;
	}

    if (can_assign && parse_match(self, token_assign)) {
        parse_expression(self, vm);
        emit_bytes(self, curr_chunk(vm->compiler), set_op, (uint8_t)index);
    } else {
        emit_bytes(self, curr_chunk(vm->compiler), get_op, (uint8_t)index);
    }
}

/* Statement:
*   - printStmt: "print" expression ";"
*   - exprStmt: expression ";"
*   - blockStmt: "{" Declaration* "}"
*/
static void parse_statement(Parser* self, VirtualMachine* vm) {
#ifndef deprecated_print_keyword
	if (parse_match(self, token_print)) {
		parse_print_statement(self, vm);
	} else
#endif
    if (parse_match(self, token_for)) {
		parse_for_statement(self, vm);
	} else if (parse_match(self, token_if)) {
		parse_if_statement(self, vm);
	} else if (parse_match(self, token_return)) {
		parse_return_statement(self, vm);
	} else if (parse_match(self, token_break)) {
		parse_break_statement(self, vm);
	} else if (parse_match(self, token_continue)) {
        parse_continue_statement(self, vm);
    } else if (parse_match(self, token_match)) {
        parse_match_statement(self, vm);
    } else if (parse_match(self, token_while)) {
		parse_while_statement(self, vm);
	} else if (parse_match(self, token_loop)) {
        parse_loop_statement(self, vm);
    }else if (parse_match(self, token_left_brace)) {
		begin_scope(vm->compiler);
		parse_block_statement(self, vm);
		end_scope(self, vm->compiler, curr_chunk(vm->compiler));
	} else {
		parse_expr_statement(self, vm);
	}
}

/*
 * breakStmt: "break" ";"
 *
 * emit break instruction.
 * while(condition) {           <- label start
 *      if (condition) {
 *          break;              -> jump to label end
 *      }                                 |
 * }                            <- label end
*/
static void parse_break_statement(Parser* self, VirtualMachine* vm) {
    parse_consume(self, token_semicolon, "[Parser::parse_break_statement] Expected ';' after 'break'.");
    // Check if we are inside a loop
    if (vm->compiler->loop == NULL) {
        parse_error_at_curr(self, "[Parser::parse_break_statement] Break statement must be inside a loop.");
        return;
    }
    if (vm->compiler->loop->end == -1) {
        // Emit a jump instruction with a placeholder offset
        int end = emit_jump(self, curr_chunk(vm->compiler), op_break);

        // Store the break jump offset for later patching
        Loop* loop = vm->compiler->loop;
        loop->end = end;
    }
}

static void parse_continue_statement(Parser* self, VirtualMachine* vm) {
    parse_consume(self, token_semicolon, "[Parser::parse_continue_statement] Expected ';' after 'continue'.");
    // Check if we are inside a loop
    if (vm->compiler->loop == NULL) {
        parse_error_at_curr(self, "[Parser::parse_continue_statement] Continue statement must be inside a loop.");
        return;
    }
    emit_continue(self, curr_chunk(vm->compiler), vm->compiler->loop->start);
}

/*
 * matchStmt: "match" expression "{" (expr => Stmt | _ => Stmt) "}"
 *
 * var i: i32 = 1;
 * match(i) {
 *   1 => print("one"),         |
 *   _ => print("other"),       ↓ (order) (default: 默认情况是所有其他arms 都不符合后最后跳转的)
 * }
 */
static void parse_match_statement(Parser* self, VirtualMachine* vm) {
    begin_scope(vm->compiler);

    if (parse_match(self, token_left_paren)) {
        parse_expression(self, vm);
        parse_consume(self, token_right_paren,
                      "[Parser::parse_match_statement] Expected ')' after match expression.");
    } else {
        parse_expression(self, vm);
    }

    Match match;
    match.start = emit_jump(self, curr_chunk(vm->compiler), op_match);
    match.stack_count = 0;
    match.has_default = false;
    parse_consume(self, token_left_brace,
                  "[Parser::parse_match_statement] Expected '{' after match expression.");

    while (!parse_check(self, token_right_brace) && !parse_check(self, token_eof)) {
        if (parse_match(self, token_underscore)) {
            begin_scope(vm->compiler);
            parse_consume(self, token_fat_arrow,
                          "[Parser::parse_match_statement] Expected '=>' after '_'.");
#ifndef deprecated_print_keyword
            if (parse_match(self, token_print)) {
                parse_print_statement(self, vm);
            } else
#endif
            if (parse_match(self, token_return)) {
                parse_return_statement(self, vm);
            } else if (parse_match(self, token_match)) {
                parse_match_statement(self, vm);
            } else if (parse_match(self, token_loop)) {
                parse_loop_statement(self, vm);
            } else if (parse_match(self, token_left_brace)) {
                parse_block_statement(self, vm);
            } else {
                parse_expr_statement(self, vm);
            }

            // has default pattern and store pattern jump to match end
            match.has_default = true;
            match.stack[match.stack_count++] = emit_jump(self, curr_chunk(vm->compiler), op_jump);
            end_scope(self, vm->compiler, curr_chunk(vm->compiler));
            parse_match(self, token_comma);
            break;
        } else {
            parse_match_member(self, vm, &match);
            parse_match(self, token_comma);
        }
        parse_match(self, token_comma);
        if(match.stack_count >= uint8_count) {
            panic("{PANIC} [parser::parse_match_statement] Match statement has too many patterns.");
        }
    }

    // If no default pattern, jump to end of match
    if (!match.has_default) {
        match.end = emit_jump(self, curr_chunk(vm->compiler), op_jump);
        patch_jump(self, curr_chunk(vm->compiler), match.start);
        match.start = match.end;
    }

    // Jump to end of match
    patch_jump(self, curr_chunk(vm->compiler), match.start);

    // Patch all pattern jumps
    for(int i = 0; i < match.stack_count; i++) {
        patch_jump(self, curr_chunk(vm->compiler), match.stack[i]);
    }

    // Pop the matched value
    emit_byte(self, curr_chunk(vm->compiler), op_pop);
    parse_consume(self, token_right_brace,
                  "[Parser::parse_match_statement] Expected '}' after match cases.");

    end_scope(self, vm->compiler, curr_chunk(vm->compiler));
}

static void parse_match_member(Parser* self, VirtualMachine* vm, Match* match) {
    begin_scope(vm->compiler);
    int jump_if_not_matched;

    if (self->curr->type == token_identifier                           // token_identifier
    && self->curr->next->type == token_layer                           // token_layer             ::
    && self->curr->next->next->type == token_identifier                // token_identifier
    ) {       // token_left_paren        (
        parse_advance(self);            // token_identifier
        parse_named_variable(self, vm, self->prev, false);
        parse_advance(self);            // token_layer ::
        parse_advance(self);            // token_identifier

        // pattern not matched jump to next pattern
        index_t enum_member_index = identifier_constant(self, vm, self->prev);
        emit_bytes(self, curr_chunk(vm->compiler), op_enum_get_member, enum_member_index);
        jump_if_not_matched = emit_jump(self, curr_chunk(vm->compiler), op_enum_member_match);

        if (parse_match(self, token_left_paren)) {
            // TODO: Bind parameter to variable
            int bind_count = 0;
            Token* counter = self->curr;
            while(counter->type == token_identifier) {
                bind_count++;
                counter = counter->next;
                if (counter->type == token_comma) {
                    counter = counter->next;
                }
            }

            index_t pattern_index[bind_count], position = 0;
            while(parse_match(self, token_identifier)) {        // identifier, identifier, ...
                index_t param_index = identifier_constant(self, vm, self->prev);
                declare_variable(self, vm->compiler, self->prev);
                define_variable(self, vm->compiler, param_index);
                pattern_index[position++] = resolve_local(self, vm->compiler, self->prev);
                if (parse_check(self, token_comma)) {
                    parse_advance(self);
                }
            }

            emit_bytes(self, curr_chunk(vm->compiler), op_enum_member_bind, bind_count);
            for(int i = 0; i < bind_count; i++) {
                emit_byte(self, curr_chunk(vm->compiler), pattern_index[i]);
            }
            parse_consume(self, token_right_paren,
                          "[Parser::parse_match_member] Expected ')' after match pattern arguments.");
        }
    } else {
        parse_expression(self, vm);
        jump_if_not_matched = emit_jump(self, curr_chunk(vm->compiler), op_jump_if_neq);
    }


    parse_consume(self, token_fat_arrow,
                  "[Parser::parse_match_member] Expected '=>' after match pattern.");

#ifndef deprecated_print_keyword
    if (parse_match(self, token_print)) {
        parse_print_statement(self, vm);
    } else
#endif
    if (parse_match(self, token_return)) {
        parse_return_statement(self, vm);
    } else if (parse_match(self, token_match)) {
        parse_match_statement(self, vm);
    } else if (parse_match(self, token_loop)) {
        parse_loop_statement(self, vm);
    } else if (parse_match(self, token_left_brace)) {
        parse_block_statement(self, vm);
    } else {
        parse_expr_statement(self, vm);
    }

    // store pattern jump to match end (jump to end of match)
    match->stack[match->stack_count++] = emit_jump(self, curr_chunk(vm->compiler), op_jump);
    patch_jump(self, curr_chunk(vm->compiler), jump_if_not_matched);
    end_scope(self, vm->compiler, curr_chunk(vm->compiler));
}


// returnStmt: "return" expression? ";"
static void parse_return_statement(Parser* self, VirtualMachine* vm) {
	// check return keyword, don't use in top-level code.
	if (vm->compiler->fn_type == type_script) {
		parse_error_at_curr(self, "[Parser::parse_return_statement] Expected return in function, Found return in top-level code.");
		return;
	}
	if (!parse_match(self, token_semicolon)) {
		parse_expression(self, vm);
		parse_consume(self, token_semicolon, "[Parser::parse_return_statement] Expected ';' after return value.");
	} else {
        // implicit return null
        if (vm->compiler->fn_type == type_initializer) {
            parse_error_at_curr(self, "[Parser::parse_return_statement] Can't return a value from an initializer.");
        }
    }
	emit_return(self, curr_chunk(vm->compiler));
}

// forStmt: for (varDecl | exprStmt | ; condition ; increment) statement;
/*
*	for
*    |     (
*    |     varDecl | exprStmt
*    |     ;
*    |     condition           <- loop_start (exit_jump)            <-           <-   <- continue_jump
*    |     ;					|                                    |            |
*    |     increment            |							<- increment_start    |
*    |     )					|							 ↑                    |
*    |     statement;			|— body_jump --------------> -                    |
*	 |     			            |- exit_jump -----------------------------------> -
*    |     <- end
*
*/
static void parse_for_statement(Parser* self, VirtualMachine* vm) {
	begin_scope(vm->compiler);

	parse_consume(self, token_left_paren, "[Parser::parse_for_statement] Expected '(' after 'for'.");
	if (parse_match(self, token_semicolon)) {
		// no initialization.
	}
	else if (parse_match(self, token_var)) {
		parse_var_declaration(self, vm);
	}
	else {
		parse_expr_statement(self, vm);
	}

	// condition: expression
	int loop_start = curr_chunk(vm->compiler)->count;
	int exit_jump = -1;
	if (!parse_match(self, token_semicolon)) {
		parse_expression(self, vm);
		parse_consume(self, token_semicolon, "[Parser::parse_for_statement] Expected ';' after loop condition.");

		// jump to the end of the loop if the condition is false.
		exit_jump = emit_jump(self, curr_chunk(vm->compiler), op_jump_if_false);
		emit_byte(self, curr_chunk(vm->compiler), op_pop); // condition
	}

    // loop{break; continue}
    Loop curr_loop;

	// increment: expression; unconditional jump that hops over the increment clause’s code to the body of the loop.
	if (!parse_match(self, token_right_paren)) {
		int body_jump = emit_jump(self, curr_chunk(vm->compiler), op_jump);
		int increment_start = curr_chunk(vm->compiler)->count; // label for the increment clause.

		parse_expression(self, vm);
		emit_byte(self, curr_chunk(vm->compiler), op_pop); // increment
		parse_consume(self, token_right_paren, "[Parser::parse_for_statement] Expected ')' after for clauses.");

		emit_loop(self, curr_chunk(vm->compiler), loop_start);
		loop_start = increment_start;

        begin_loop(vm->compiler, &curr_loop, loop_start);
		patch_jump(self, curr_chunk(vm->compiler), body_jump);
	}

    // statement: statement;
	parse_statement(self, vm);

	// emit loop op_loop
	emit_loop(self, curr_chunk(vm->compiler), loop_start);
	// patch exit_jump to here: update loop chunk [opcode, jump_offset, jump_offset]
	if (exit_jump != -1) {
		patch_jump(self, curr_chunk(vm->compiler), exit_jump);
		emit_byte(self, curr_chunk(vm->compiler), op_pop); // condition
	}

    if (vm->compiler->loop->end != -1) {
        emit_break(self, curr_chunk(vm->compiler));
    }

    // end loop
    end_loop(vm->compiler);
	end_scope(self, vm->compiler, curr_chunk(vm->compiler));
}

// whileStmt: while (expression) statement;
/*
*	while
*    |     <- loop_start  <-——--- loop_start   <- continue_jump
*    |     (				↑
*    |     condition		|
*    |     )				|
*    |     <- exit_jump		|
*    |     {				|
*    |         statement	|
*    |     }				|
*    |     <--- op_loop --->|
*    |        | loop_offset |
*    |        V loop_offset | <- chunck->count
*    |     <- end
*	=> offset = chunk->count - loop_start + 2(loop_offset)
*/
static void parse_while_statement(Parser* self, VirtualMachine* vm) {
	int loop_start = curr_chunk(vm->compiler)->count;

    if (parse_match(self, token_left_paren)) {
        parse_expression(self, vm);
        parse_consume(self, token_right_paren, "[Parser::parse_while_statement] Expected ')' after condition.");
    } else {
        parse_expression(self, vm);
    }

    Loop curr_loop;
    begin_loop(vm->compiler, &curr_loop, loop_start);

	int exit_jump = emit_jump(self, curr_chunk(vm->compiler), op_jump_if_false);
	emit_byte(self, curr_chunk(vm->compiler), op_pop);
	parse_statement(self, vm);
	emit_loop(self, curr_chunk(vm->compiler), loop_start);

	patch_jump(self, curr_chunk(vm->compiler), exit_jump);
	emit_byte(self, curr_chunk(vm->compiler), op_pop);

    if (vm->compiler->loop->end != -1) {
        emit_break(self, curr_chunk(vm->compiler));
    }
    end_loop(vm->compiler);
}

/*
 * loopStmt: loop {}  => while(true) {}
 *
 * loop Stmt => while(true) Stmt
 * loop {           <- loop_start
 *                          ^
 *  if (cond) {             |
 *      continue;  <- jump_start
 *  }
 *  if (cond) {
 *      break;      <- jump_start
 *  }                       |
 *                          v
 * }                <- jump_end
 */
static void parse_loop_statement(Parser* self, VirtualMachine* vm) {

    Loop curr_loop;
    begin_loop(vm->compiler, &curr_loop, curr_chunk(vm->compiler)->count);
    parse_statement(self, vm);
    emit_loop(self, curr_chunk(vm->compiler), curr_loop.start);

    if (vm->compiler->loop->end != -1) {
        emit_break(self, curr_chunk(vm->compiler));
    }
    end_loop(vm->compiler);
}

// ifStmt: if (expression) statement else statement;
static void parse_if_statement(Parser* self, VirtualMachine* vm) {

    if (parse_match(self, token_left_paren)) {
        parse_expression(self, vm);
        parse_consume(self, token_right_paren, "[Parser::parse_if_statement] Expected ')' after condition.");
    } else {
        parse_expression(self, vm);
    }

	// backpacking later: chunk [opcode, temp slot, temp slot]
	int then_jump = emit_jump(self, curr_chunk(vm->compiler), op_jump_if_false);
	emit_byte(self, curr_chunk(vm->compiler), op_pop);
	parse_statement(self, vm);

	// backpacking later: chunk [opcode, temp slot, temp slot]
	int else_jump = emit_jump(self, curr_chunk(vm->compiler), op_jump);
	// patch then_jump to here: update then chunk [opcode, jump_offset, jump_offset]
	patch_jump(self, curr_chunk(vm->compiler), then_jump);

	emit_byte(self, curr_chunk(vm->compiler), op_pop);
	if (parse_match(self, token_else)) parse_statement(self, vm);

	// patch else_jump to here: update else chunk [opcode, jump_offset, jump_offset]
	patch_jump(self, curr_chunk(vm->compiler), else_jump);
}

static void parse_block_statement(Parser* self, VirtualMachine* vm) {
	while (!parse_check(self, token_right_brace) && !parse_check(self, token_eof)) {
		parse_declaration(self, vm);
	}
	parse_consume(self, token_right_brace, "[Parser::parse_block_statement] Expected '}' after block statement.");
}

static __attribute__((unused)) void parse_print_statement(Parser* self, VirtualMachine* vm) {
	parse_expression(self, vm);
	parse_consume(self, token_semicolon, "[Parser::parse_print_statement] Expected ';' after expression.");
	emit_byte(self, curr_chunk(vm->compiler), op_print);
}

static void parse_expr_statement(Parser* self, VirtualMachine* vm) {
	parse_expression(self, vm);
    parse_match(self, token_semicolon);
	emit_byte(self, curr_chunk(vm->compiler), op_pop);
}

static void parse_expression(Parser* self, VirtualMachine* vm) {
	parse_precedence(self, vm, prec_assign);
}

static void parse_precedence(Parser* self, VirtualMachine* vm, Precedence outer_precedence) {
	/*
	* execute advance() after:
	*	'Scanner->curr' will move to 'Scanner->prev'
	*   Scanner:
	*       -----(move) ----->
	*	type: curr         next
	*		|            |
	*		V		     V
	*		prev(1), curr(2), next(3), ...
	*/
	if (parse_advance(self) == NULL) {
		parse_error_at_prev(self, "[Parser::parse_precedence] Expected expression, Found end of file.");
		return;
	}

	/* get curr handle token's prefix rule */
	ParseFnPtr prev_prefix_rule = get_rule(self->prev->type)->prefix;
	if (prev_prefix_rule == NULL) {
		parse_error_at_prev(self, "[Parser::parse_precedence] Expected prefix rule for this token, Found NULL.");
		return;
	}

	/* dispatch to prefix rule */
	bool can_assign = outer_precedence <= prec_assign;
	prev_prefix_rule(self, vm, can_assign);

	/* get curr next token's infix rule:
	*    1. if next token's precedence is greater than curr token's precedence,
	*       then execute infix rule of next token.
	*    2. if next token's precedence is less than or equal to curr token's precedence,
	*       then execute infix rule of curr token.
	*
	* precedence:
	*     1. parameter: outsiders' precedence.
	*/

	/* check if there is a next token, if is not the end of file */
	if (parse_is_at_end(self)) return;

	while (outer_precedence <= get_rule(self->curr->type)->precedence[1]) {
		/*
		*   Scanner:
		*   -------------------- (move) -------------------->
		*   type: prev	              curr         next
		*	    |			          |            |
		*	    V			          V		       V
		*	    prev_prev(1), prev(2), curr(3), next(4), ...
		*/
		if (parse_advance(self) == NULL) {
			parse_error_at_prev(self, "[Parser::parse_precedence] Expected expression, Found end of file.");
			return;
		}

		/* get curr handle token's infix rule */
		ParseFnPtr prev_infix_rule = get_rule(self->prev->type)->infix;
		prev_infix_rule(self, vm, can_assign);
	}

	/* check if is assignment operator, if is, then error */
	if (can_assign && parse_match(self, token_assign)) {
		parse_error_at_curr(self, "[Parser::parse_precedence] Expected expression, Found assignment operator.");
	}
}

/* parse number: {i32, f64,...}, need detailed dispatching */
/* TODO: add more type */
void parse_i32(Parser* self, VirtualMachine* vm, bool _can_assign) {
    (void)_can_assign;

	// int32_t value = atoi(self->prev->start);
    int32_t value = strtol(self->prev->start, NULL, 10);
	emit_constant(self, curr_chunk(vm->compiler), macro_val_from_i32(value));
}
void parse_i64(Parser* self, VirtualMachine* vm, bool _can_assign) {
    (void)_can_assign;

	// int64_t value = atoll(self->prev->start);
    int64_t value = strtoll(self->prev->start, NULL, 10);
	emit_constant(self, curr_chunk(vm->compiler), macro_val_from_i64(value));
}
void parse_f32(Parser* self, VirtualMachine* vm, bool _can_assign) {
    (void)_can_assign;

	float value = strtof(self->prev->start, NULL);
	emit_constant(self, curr_chunk(vm->compiler), macro_val_from_f32(value));
}
void parse_f64(Parser* self, VirtualMachine* vm, bool _can_assign) {
    (void)_can_assign;

	double value = strtod(self->prev->start, NULL);
	emit_constant(self, curr_chunk(vm->compiler), macro_val_from_f64(value));
}

void parse_grouping(Parser* self, VirtualMachine* vm, bool _can_assign) {
    (void)_can_assign;

	parse_expression(self, vm);
	parse_consume(self, token_right_paren, "[Parser::parse_grouping] Expected ')' after expression.");
}

static uint8_t parse_argument_list(Parser* self, VirtualMachine* vm) {
	uint8_t arg_count = 0;
	if (!parse_check(self, token_right_paren)) {
		do {
			parse_expression(self, vm);
			if (arg_count == argument_count_max) {
				parse_error_at_curr(self, "[Parser::parse_argument_list] Too many arguments, limit is 255.");
			}
			arg_count++;
		} while (parse_match(self, token_comma));
	}
	parse_consume(self, token_right_paren, "[Parser::parse_argument_list] Expected ')' after arguments.");
	return arg_count;
}

/*
* parse function call:
* {..., [op_call, arg_count], ... }
*/
void parse_call(Parser* self, VirtualMachine* vm, bool _can_assign) {
    (void)_can_assign;

	uint8_t arg_count = parse_argument_list(self, vm);
	emit_bytes(self, curr_chunk(vm->compiler), op_call, arg_count);
}

/* previous suffix expression: -a.b + c; => parse_precedence ; 嵌套一元表达式 */
void parse_unary(Parser* self, VirtualMachine* vm, bool _can_assign) {
    (void)_can_assign;

	TokenType operator_type = self->prev->type;

	// compile the operand
	parse_precedence(self, vm, prec_unary);

	// emit the operator instruction
	switch (operator_type) {
	case token_not:    emit_byte(self, curr_chunk(vm->compiler), op_not); break;
	case token_minus:   emit_byte(self, curr_chunk(vm->compiler), op_negate); break;
	default:            panic("[Parser::parse_unary] %s Unreachable.",
                              macro_token_type_to_string(operator_type));
	}
}

/* middle suffix expression: a + b */
void parse_binary(Parser* self, VirtualMachine* vm, bool _can_assign) {
    (void)_can_assign;

	TokenType operator_type = self->prev->type;
	ParseRule* rule = get_rule(operator_type);

	// compile the right operand
	parse_precedence(self, vm, (Precedence)(rule->precedence[0] + 1));

	// emit the operator instruction
	switch (operator_type) {
	case token_neq:	emit_byte(self, curr_chunk(vm->compiler), op_not_equal); break;
	case token_eq:  emit_byte(self, curr_chunk(vm->compiler), op_equal); break;
	case token_gt:  emit_byte(self, curr_chunk(vm->compiler), op_greater); break;
	case token_egt:	emit_byte(self, curr_chunk(vm->compiler), op_greater_equal); break;
	case token_le:  emit_byte(self, curr_chunk(vm->compiler), op_less); break;
	case token_let: emit_byte(self, curr_chunk(vm->compiler), op_less_equal); break;
	case token_plus:    emit_byte(self, curr_chunk(vm->compiler), op_add); break;
	case token_minus:   emit_byte(self, curr_chunk(vm->compiler), op_subtract); break;
	case token_star:    emit_byte(self, curr_chunk(vm->compiler), op_multiply); break;
    case token_slash:   emit_byte(self, curr_chunk(vm->compiler), op_divide); break;
    case token_mod:     emit_byte(self, curr_chunk(vm->compiler), op_mod); break;
    default:    panic("[Parser::parse_unary] %s Unreachable.", macro_token_type_to_string(operator_type));
	}
}

void parse_literal(Parser* self, VirtualMachine* vm, bool _can_assign) {
    (void)_can_assign;

	switch (self->prev->type) {
	case token_false: emit_byte(self, curr_chunk(vm->compiler), op_false); break;
	case token_true:  emit_byte(self, curr_chunk(vm->compiler), op_true);  break;
	case token_none:  emit_byte(self, curr_chunk(vm->compiler), op_none);  break;
	default: panic("[Parser::parse_literal] Unreachable.");
	}
}
/* parse string literal */
void parse_string(Parser* self, VirtualMachine* vm, bool _can_assign) {
    (void)_can_assign;

	emit_constant(self, curr_chunk(vm->compiler), macro_val_from_obj(new_string(
		vm,
		self->prev->start + 1,
		self->prev->length - 2
	)));
}

void parse_and(Parser* self, VirtualMachine* vm, bool _can_assign) {
    (void)_can_assign;

	int end = emit_jump(self, curr_chunk(vm->compiler), op_jump_if_false);
	emit_byte(self, curr_chunk(vm->compiler), op_pop);
	parse_precedence(self, vm, prec_and);
	patch_jump(self, curr_chunk(vm->compiler), end);
}

void parse_or(Parser* self, VirtualMachine* vm, bool _can_assign) {
    (void)_can_assign;

	int else_jump = emit_jump(self, curr_chunk(vm->compiler), op_jump_if_false);
	int end = emit_jump(self, curr_chunk(vm->compiler), op_jump);
	patch_jump(self, curr_chunk(vm->compiler), else_jump);
	emit_byte(self, curr_chunk(vm->compiler), op_pop);
	parse_precedence(self, vm, prec_or);
	patch_jump(self, curr_chunk(vm->compiler), end);
}

void parse_bitwise_and(Parser* self, VirtualMachine* vm, bool can_assign) {
    (void)can_assign;

    parse_precedence(self, vm, prec_bitwise_and);
    emit_byte(self, curr_chunk(vm->compiler), op_bw_and);
}

void parse_bitwise_or(Parser* self, VirtualMachine* vm, bool can_assign) {
    (void)can_assign;

    parse_precedence(self, vm, prec_bitwise_or);
    emit_byte(self, curr_chunk(vm->compiler), op_bw_or);
}

void parse_bitwise_xor(Parser* self, VirtualMachine* vm, bool can_assign) {
    (void)can_assign;

    parse_precedence(self, vm, prec_bitwise_xor);
    emit_byte(self, curr_chunk(vm->compiler), op_bw_xor);
}

void parse_shift_left(Parser* self, VirtualMachine* vm, bool can_assign) {
    (void)can_assign;

    parse_precedence(self, vm, prec_shift);
    emit_byte(self, curr_chunk(vm->compiler), op_bw_sl);
}

void parse_shift_right(Parser* self, VirtualMachine* vm, bool can_assign) {
    (void)can_assign;

    parse_precedence(self, vm, prec_shift);
    emit_byte(self, curr_chunk(vm->compiler), op_bw_sr);
}

void parse_bitwise_not(Parser* self, VirtualMachine* vm, bool can_assign) {
    (void)can_assign;

    parse_precedence(self, vm, prec_unary);
    emit_byte(self, curr_chunk(vm->compiler), op_bw_not);
}

void parse_dot(Parser* self, VirtualMachine* vm, bool can_assign) {
    parse_consume(self, token_identifier, "[Parser::parse_dot] Expected property name after '.'.");
    index_t  property_index = identifier_constant(self, vm, self->prev);

    Chunk *curr_ck = curr_chunk(vm->compiler);
    // optional assignment
    if (can_assign && parse_match(self, token_assign)) {
        parse_expression(self, vm);
        emit_bytes(self, curr_ck, op_set_property, property_index);
    }
    // optional call
    else if (parse_match(self, token_left_paren)) {
        index_t arg_count = parse_argument_list(self, vm);
        emit_bytes(self, curr_ck, op_invoke, property_index);
        emit_byte(self, curr_ck, arg_count);
    }
    // get property
    else {
        emit_bytes(self, curr_ck, op_get_property, property_index);
    }
}

void parse_layer(Parser* self, VirtualMachine* vm, bool _can_assign) {
    (void)_can_assign;

    parse_consume(self, token_identifier, "[Parser::parse_layer] Expected layer property name after '::'.");
    index_t property_index = identifier_constant(self, vm, self->prev);

    Chunk* chunk = curr_chunk(vm->compiler);

    // 检查是否可能为类或枚举的静态方法调用
    if (parse_match(self, token_left_paren)) {
        uint8_t arg_count = parse_argument_list(self, vm);
        emit_bytes(self, chunk, op_layer_property_call, property_index);
        emit_byte(self, chunk, arg_count);
    } else {
        // 静态属性访问或枚举成员
        emit_bytes(self, chunk, op_get_layer_property, property_index);
    }
}


// a += 1；-=
// classInstance.a += 1； -=
// structInstance.a += 1；-=
void parse_compound_assign(Parser* self, VirtualMachine* vm, bool can_assign) {
    if (!can_assign) {
        parse_error_at_curr(self, "[Parser::parse_compound_assign] Invalid assignment target.");
        return;
    }

    // 左值 && 运算符类型 && 解析右表达式
    Token* var_name = self->pprev;
    TokenType operator_type = self->prev->type;
    parse_precedence(self, vm, prec_assign);

    // 生成运算指令
    Chunk* chunk = curr_chunk(vm->compiler);
    switch (operator_type) {
        case token_plus_assign: emit_byte(self, chunk, op_add); break;
        case token_minus_assign:emit_byte(self, chunk, op_subtract); break;
        case token_star_assign: emit_byte(self, chunk, op_multiply); break;
        case token_slash_assign:emit_byte(self, chunk, op_divide); break;
        case token_shl_assign:  emit_byte(self, chunk, op_bw_sl); break;
        case token_shr_assign:  emit_byte(self, chunk, op_bw_sr); break;
        case token_bit_and_assign: emit_byte(self, chunk, op_bw_and); break;
        case token_bit_or_assign:  emit_byte(self, chunk, op_bw_or); break;
        case token_bit_xor_assign: emit_byte(self, chunk, op_bw_xor); break;
        case token_bit_not_assign: emit_byte(self, chunk, op_bw_not); break;
        case token_mod_assign:  emit_byte(self, chunk, op_mod); break;
        default: parse_error_at_curr(self, "[Parser::parse_compound_assign] Unsupported compound operator."); break;
    }

    // 生成存储指令（将运算结果存回左值）
    int index = resolve_local(self, vm->compiler, var_name);
    if (index != -1) {
        emit_bytes(self, chunk, op_set_local, (uint8_t)index);
    } else if ((index = resolve_upvalue(self, vm->compiler, var_name)) != -1) {
        emit_bytes(self, chunk, op_set_upvalue, (uint8_t)index);
    } else {
        index = identifier_constant(self, vm, var_name);
        emit_bytes(self, chunk, op_set_global, (uint8_t)index);
    }
}

/* ternary expr: expr ? expr : expr; */
void parse_ternary(Parser* self, VirtualMachine* vm, bool _can_assign) {
    (void)_can_assign;

    // 解析条件后的真分支
    int then_jump = emit_jump(self, curr_chunk(vm->compiler), op_jump_if_false);
    emit_byte(self, curr_chunk(vm->compiler), op_pop); // 弹出条件结果

    // 解析真值表达式
    parse_precedence(self, vm, prec_assign);
    int else_jump = emit_jump(self, curr_chunk(vm->compiler), op_jump);

    // 解析假分支
    patch_jump(self, curr_chunk(vm->compiler), then_jump);
    emit_byte(self, curr_chunk(vm->compiler), op_pop); // 弹出条件结果
    parse_consume(self, token_colon, "Expected ':' in ternary expression");
    parse_precedence(self, vm, prec_assign);

    patch_jump(self, curr_chunk(vm->compiler), else_jump);
}

/* 先判断 是否是 lambda 表达式， 不是 则是 bit or 调用相应的函数并传递 优先级
 * lambda: => |a: i32, b: f64| -> f64 {}
 * */
void parse_lambda(Parser* self, VirtualMachine* vm, bool _can_assign) {
    (void)_can_assign;

    Compiler lambda_compiler;
    curr_into_sub_compiler(vm, &lambda_compiler, type_lambda, self);
    begin_scope(vm->compiler);

    // 解析参数列表
    if (!parse_match(self, token_pipe)) {
        do {
            vm->compiler->fn->arity++;
            if (vm->compiler->fn->arity > argument_count_max) {
                parse_error_at_curr(self, "Too many lambda parameters");
            }

            uint8_t param_index = parse_variable(self, vm, "Expected parameter name");

            // 解析类型注解
            if (parse_match(self, token_colon)) {
                parse_consume(self, token_identifier, "Expected type annotation");
                // 类型信息可存储到符号表中
            }

            define_variable(self, vm->compiler, param_index);
        } while (parse_match(self, token_comma));
        parse_consume(self, token_pipe, "Expected '|' after parameters");
    }

    // 解析返回类型
    if (parse_match(self, token_arrow)) {
        parse_consume(self, token_identifier, "Expected return type");
        // 存储返回类型信息
    }

    // 解析函数体
    if(parse_match(self, token_left_brace)){
        parse_block_statement(self, vm);
    } else {
        parse_expression(self, vm);
    }

    // 生成闭包指令
    Fn* lambda_fn = curr_from_sub_compiler(self, vm);
    index_t constant = make_constant(self, curr_chunk(vm->compiler),
                                   macro_val_from_obj(lambda_fn));
    emit_bytes(self, curr_chunk(vm->compiler), op_closure, constant);

    // 处理upvalues
    for (int i = 0; i < lambda_fn->upvalue_count; i++) {
        emit_byte(self, curr_chunk(vm->compiler), lambda_compiler.upvalues[i]);
    }
}




/*
 * 解析向量语法
 */
void parse_vector(Parser* self, VirtualMachine* vm, bool _can_assign) {
    (void)_can_assign;

    // 解析向量元素并生成对应的字节码
    int element_count = 0;
    while(!parse_check(self, token_right_bracket)) {
        parse_expression(self, vm);
        element_count++;
        if (!parse_match(self, token_comma)) break;
    }

    // 确保闭合右括号
    parse_consume(self, token_right_bracket, "[Parser::parse_vector] Expected ']' after vector elements.");
    // 生成op_vector_new指令，传入元素数量
    emit_bytes(self, curr_chunk(vm->compiler), op_vector_new, element_count);
}

void parse_vector_index(Parser* self, VirtualMachine* vm, bool _can_assign) {

    // 解析索引表达式，例如 vec[0]
    parse_expression(self, vm);
    parse_consume(self, token_right_bracket, "[Parser::parse_vector_index] Expected ']' after vector index.");

    // 生成字节码：读取或写入元素
    if(_can_assign && parse_match(self, token_assign)) {
        parse_expression(self, vm);
        emit_byte(self, curr_chunk(vm->compiler), op_vector_set);
    } else {
        emit_byte(self, curr_chunk(vm->compiler), op_vector_get);
    }
}


void parse_error_at_curr(Parser* self, const char* message) {
	if (self->curr == NULL) {
		parse_error_at_prev(self, "[Parser::parse_error_at_curr] Expected expression, Found end of file.");
		return;
	}
	parse_error_at(self, self->curr, message);
}

void parse_error_at_prev(Parser* self, const char* message) {
	if (self->prev == NULL) {
		parse_error_at(self, self->tokens->head, "[Parser::parse_error_at_prev] Expected expression, Found end of file.");
		return;
	}
	parse_error_at(self, self->prev, message);
}

/*
* PS:
*	[line 1] where: {'at end'" | at 'lexeme'} .
*		msg: 'parse error message'.
*
*/
static void parse_error_at(Parser* self, Token* error, const char* message) {
	/* if had error, then return */
	if (self->panic_mode) return;
	/* set panic mode */
	self->panic_mode = true;

	fprintf(stderr, "[line %d] ", error->line);
	switch (error->type)
	{
	case token_eof: fprintf(stderr, "where: at end.\n"); break;
	case token_error: fprintf(stderr, "where: at error.\n"); break;
	default: fprintf(stderr, "where: at '%.*s'.\n", error->length, error->start); break;
	}
	fprintf(stderr, "\tmsg: %s\n", message);

	// TODO: handler error
	self->had_error = true;
}
