//
// Created by Kilig on 2024/11/21.
//
#pragma once

#ifndef JOKER_PARSER_H
#define JOKER_PARSER_H
#include "common.h"
#include "token.h"

typedef struct Parser {
    VirtualMachine *vm;
	TokenList* tokens;
	Token* curr;
    Token* prev;
    Token* pprev;
	bool had_error;
	bool panic_mode;
} Parser;

void init_parser(Parser* self, VirtualMachine *vm, TokenList* tokens);
void free_parser(Parser* self);
void print_parser(Parser* self);

/* Precedence levels : lowest to highest */
typedef enum Precedence {
    prec_none,          // No precedence
    prec_assign,        // = += -= *= /= %= 等赋值操作
    prec_conditional,   // ? :
    prec_or,            // 逻辑或 ||
    prec_and,           // 逻辑与 &&
    prec_bitwise_or,    // 按位或 |
    prec_bitwise_xor,   // 按位异或 ^
    prec_bitwise_and,   // 按位与 &
    prec_equality,      // == !=
    prec_comparison,    // < > <= >=
    prec_shift,         // << >>
    prec_term,          // + -
    prec_factor,        // * / %
    prec_unary,         // 单目运算符 ! ~ -
    prec_call,          // 函数调用 () [] . 层序访问 ::
    prec_primary        // 字面量、变量名
} Precedence;


typedef void (*ParseFnPtr)(Parser* self, VirtualMachine* vm, bool can_assign);			// ParseFn -point-> void function()

/* parser table of rules row */
typedef struct ParseRule {
	ParseFnPtr prefix;
	ParseFnPtr infix;
	Precedence precedence[2];
} ParseRule;
#define MACRO_PREC(operator) {operator, operator}


/* parser function prototypes */
Fn* parse_tokens(Parser* self, VirtualMachine* vm);
void parse_grouping(Parser* self, VirtualMachine* vm, bool can_assign);
void parse_call(Parser* self, VirtualMachine* vm, bool can_assign);
void parse_dot(Parser* self, VirtualMachine* vm, bool can_assign);
void parse_layer(Parser* self, VirtualMachine* vm, bool can_assign);
void parse_i32(Parser* self, VirtualMachine* vm, bool _can_assign);
void parse_i64(Parser* self, VirtualMachine* vm, bool _can_assign);
void parse_f32(Parser* self, VirtualMachine* vm, bool _can_assign);
void parse_f64(Parser* self, VirtualMachine* vm, bool _can_assign);
void parse_unary(Parser* self, VirtualMachine* vm, bool can_assign);
void parse_binary(Parser* self, VirtualMachine* vm, bool can_assign);
void parse_literal(Parser* self, VirtualMachine* vm, bool _can_assign);
void parse_string(Parser* self, VirtualMachine* vm, bool _can_assign);
void parse_identifier(Parser* self, VirtualMachine* vm, bool _can_assign);
void parse_and(Parser* self, VirtualMachine* vm, bool _can_assign);
void parse_or(Parser* self, VirtualMachine* vm, bool _can_assign);
void parse_self(Parser* self, VirtualMachine* vm, bool _can_assign);
void parse_super(Parser* self, VirtualMachine* vm, bool _can_assign);
void parse_vector(Parser* self, VirtualMachine* vm, bool can_assign);
void parse_vector_index(Parser* self, VirtualMachine* vm, bool can_assign);
void parse_bitwise_and(Parser* self, VirtualMachine* vm, bool can_assign);
void parse_bitwise_or(Parser* self, VirtualMachine* vm, bool can_assign);
void parse_bitwise_xor(Parser* self, VirtualMachine* vm, bool can_assign);
void parse_shift_left(Parser* self, VirtualMachine* vm, bool can_assign);
void parse_shift_right(Parser* self, VirtualMachine* vm, bool can_assign);
void parse_bitwise_not(Parser* self, VirtualMachine* vm, bool can_assign);
void parse_compound_assign(Parser* self, VirtualMachine* vm, bool can_assign);
void parse_ternary(Parser* self, VirtualMachine* vm, bool _can_assign);
void parse_lambda(Parser* self, VirtualMachine* vm, bool _can_assign);


/* parser rules table: Pattern-Action table */
static ParseRule __attribute__((unused)) static_syntax_rules[] = {
	/* entry: [index]	= {prefix,			    infix,			           infix_precedence} */
	[token_left_paren]  = {parse_grouping,	parse_call, 		    MACRO_PREC(prec_call)},
	[token_right_paren] = {NULL,			    NULL,           	 MACRO_PREC(prec_none)},
	[token_left_bracket]=  {parse_vector,	    parse_vector_index, MACRO_PREC(prec_call)},
    [token_right_bracket]= {NULL,			    NULL,           	 MACRO_PREC(prec_none)},
    [token_left_brace]  = {NULL,			    NULL,          	    MACRO_PREC(prec_none)},
	[token_right_brace] = {NULL,			    NULL,		         MACRO_PREC(prec_none)},
	[token_comma]       = {NULL,			    NULL,        	     MACRO_PREC(prec_none)},
    [token_dot]         = {NULL,			    parse_dot,		     MACRO_PREC(prec_call)},
    [token_question]    = {NULL,			    parse_ternary,      MACRO_PREC(prec_conditional)},
    [token_colon]       = {NULL,                NULL,               MACRO_PREC(prec_none)},
    [token_layer]       = {NULL,                parse_layer,	     MACRO_PREC(prec_call)},
    [token_bit_and]     = {NULL,                parse_bitwise_and,   MACRO_PREC(prec_bitwise_and)},
    [token_pipe]        = {parse_lambda,        parse_bitwise_or,    {prec_primary, prec_bitwise_or}},
    [token_bit_xor]     = {NULL,                parse_bitwise_xor,   MACRO_PREC(prec_bitwise_xor)},
    [token_bit_not]     = {parse_bitwise_not,NULL,                   MACRO_PREC(prec_unary)},
    [token_shl]         = {NULL,                parse_shift_left,    MACRO_PREC(prec_shift)},
    [token_shr]         = {NULL,                parse_shift_right,   MACRO_PREC(prec_shift)},
    [token_plus_assign]  = {NULL,               parse_compound_assign,   MACRO_PREC(prec_assign)},
    [token_minus_assign] = {NULL,               parse_compound_assign,   MACRO_PREC(prec_assign)},
    [token_star_assign]  = {NULL,               parse_compound_assign,   MACRO_PREC(prec_assign)},
    [token_slash_assign] = {NULL,               parse_compound_assign,   MACRO_PREC(prec_assign)},
    [token_mod_assign]   = {NULL,               parse_compound_assign,   MACRO_PREC(prec_assign)},
    [token_bit_and_assign] = {NULL,             parse_compound_assign,   MACRO_PREC(prec_assign)},
    [token_bit_or_assign]  = {NULL,             parse_compound_assign,   MACRO_PREC(prec_assign)},
    [token_bit_xor_assign] = {NULL,             parse_compound_assign,   MACRO_PREC(prec_assign)},
    [token_bit_not_assign] = {NULL,             parse_compound_assign,   MACRO_PREC(prec_assign)},
    [token_shl_assign]  = {NULL,                parse_compound_assign,   MACRO_PREC(prec_assign)},
    [token_shr_assign]  = {NULL,                parse_compound_assign,   MACRO_PREC(prec_assign)},
	[token_minus]       = {parse_unary,	        parse_binary,	        MACRO_PREC(prec_term)},
	[token_plus]        = {NULL,			    parse_binary,	     MACRO_PREC(prec_term)},
	[token_semicolon]   = {NULL,			    NULL,			     MACRO_PREC(prec_none)},
	[token_slash]       = {NULL,			    parse_binary,	     MACRO_PREC(prec_factor)},
    [token_star]        = {NULL,			    parse_binary,	     MACRO_PREC(prec_factor)},
    [token_mod]         = {NULL,			    parse_binary,	     MACRO_PREC(prec_factor)},
	[token_not]         = {parse_unary,	        NULL,		         MACRO_PREC(prec_unary)},
	[token_assign]      = {NULL,			    NULL,	             MACRO_PREC(prec_none)},
	[token_neq]         = {NULL,			    parse_binary,        MACRO_PREC(prec_equality)},
	[token_eq]          = {NULL,			    parse_binary,        MACRO_PREC(prec_equality)},
	[token_gt]          = {NULL,			    parse_binary,        MACRO_PREC(prec_comparison)},
	[token_egt]         = {NULL,			    parse_binary,        MACRO_PREC(prec_comparison)},
	[token_le]          = {NULL,			    parse_binary,        MACRO_PREC(prec_comparison)},
	[token_let]         = {NULL,			    parse_binary,        MACRO_PREC(prec_comparison)},
	[token_identifier]  = {parse_identifier,    NULL,			     MACRO_PREC(prec_none)},
	[token_string]      = {parse_string,	    NULL,			     MACRO_PREC(prec_none)},
	[token_i32]         = {parse_i32,		    NULL,			     MACRO_PREC(prec_none)},
	[token_i64]         = {parse_i64,		    NULL,			     MACRO_PREC(prec_none)},
	[token_f32]         = {parse_f32,		    NULL,			     MACRO_PREC(prec_none)},
	[token_f64]         = {parse_f64,		    NULL,			     MACRO_PREC(prec_none)},
	[token_and]         = {NULL,			    parse_and,		     MACRO_PREC(prec_and)},
	[token_class]       = {NULL,			    NULL,			     MACRO_PREC(prec_none)},
	[token_else]        = {NULL,			    NULL,			     MACRO_PREC(prec_none)},
	[token_false]       = {parse_literal,       NULL,			     MACRO_PREC(prec_none)},
	[token_for]         = {NULL,			    NULL,			     MACRO_PREC(prec_none)},
	[token_fn]          = {NULL,			    NULL,			     MACRO_PREC(prec_none)},
	[token_if]          = {NULL,			    NULL,			     MACRO_PREC(prec_none)},
	[token_none]        = {parse_literal,       NULL,			     MACRO_PREC(prec_none)},
	[token_or]          = {NULL,			    parse_or,		     MACRO_PREC(prec_or)},
#ifndef deprecated_print_keyword
	[token_print]       = {NULL,			    NULL,			     prec_none},
#endif
	[token_return]      = {NULL,			    NULL,			     MACRO_PREC(prec_none)},
	[token_super]       = {parse_super,	        NULL,			     MACRO_PREC(prec_none)},
	[token_self]        = {parse_self,	        NULL,			     MACRO_PREC(prec_none)},
	[token_true]        = {parse_literal,       NULL,			     MACRO_PREC(prec_none)},
	[token_var]         = {NULL,			    NULL,			     MACRO_PREC(prec_none)},
    [token_while]       = {NULL,			    NULL,			     MACRO_PREC(prec_none)},
    [token_match]       = {NULL,			    NULL,			     MACRO_PREC(prec_none)},
    [token_error]       = {NULL,			    NULL,			     MACRO_PREC(prec_none)},
	[token_eof]         = {NULL,			    NULL,			     MACRO_PREC(prec_none)}
};

/* get parser rule for token type */
ParseRule* get_rule(TokenType type);
#endif //JOKER_PARSER_H
