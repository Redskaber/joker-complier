//
// Created by Kilig on 2024/11/21.
//
#pragma once

#ifndef JOKER_TOKEN_H
#define JOKER_TOKEN_H
#include "common.h"

/* display Token type to string macro */
#define macro_token_type_to_string(type)			    \
	(type == token_left_paren) ? "token_left_paren" :   \
	(type == token_right_paren)? "token_right_paren" :	\
	(type == token_left_bracket) ? "token_left_bracket" : \
	(type == token_right_bracket)? "token_right_bracket" :\
	(type == token_left_brace) ? "token_left_brace" :   \
	(type == token_right_brace)? "token_right_brace" :	\
	(type == token_right_brace)? "token_right_brace" :	\
	(type == token_comma)     ? "token_comma" :			\
	(type == token_dot)       ? "token_dot" :		    \
	(type == token_pipe)       ? "token_pipe" :		    \
	(type == token_minus)     ? "token_minus" :			\
	(type == token_plus)      ? "token_plus" :          \
    (type == token_underscore) ? "token_underscore" :   \
    (type == token_layer)     ? "token_layer" :         \
    (type == token_colon)     ? "token_colon" :			\
	(type == token_semicolon) ? "token_semicolon" :		\
	(type == token_question) ? "token_question" :		\
	(type == token_slash)     ? "token_slash" :         \
	(type == token_mod)       ? "token_mod" :           \
    (type == token_plus_assign)? "token_plus_assign" :    \
    (type == token_minus_assign)? "token_minus_assign" :  \
    (type == token_star_assign)? "token_star_assign":     \
    (type == token_slash_assign)? "token_slash_assign":   \
    (type == token_mod_assign)? "token_mod_assign":     \
    (type == token_bit_and)? "token_bit_and":           \
    (type == token_bit_and_assign)? "token_bit_and_assign":  \
    (type == token_bit_or)? "token_bit_or":             \
    (type == token_bit_or_assign)? "token_bit_or_assign":    \
    (type == token_bit_xor)? "token_bit_xor":           \
    (type == token_bit_xor_assign)? "token_bit_xor_assign":  \
    (type == token_bit_not)? "token_bit_not":           \
    (type == token_bit_not_assign)? "token_bit_not_assign":  \
    (type == token_shl)? "token_shl":                   \
    (type == token_shl_assign)? "token_shl_assign":     \
    (type == token_shr)? "token_shr":                   \
    (type == token_shr_assign)? "token_shr_assign":     \
	(type == token_star)      ? "token_star" :			\
	(type == token_not)      ? "token_not" :			\
	(type == token_neq) ? "token_neq" :	                \
	(type == token_assign)     ? "token_assign" :			\
	(type == token_eq)? "token_eq" :	\
	(type == token_gt)   ? "token_gt" :		\
	(type == token_egt)? "token_egt" :\
	(type == token_le)      ? "token_le" :			\
	(type == token_let) ? "token_let" :	\
	(type == token_arrow)     ? "token_arrow" :			\
	(type == token_fat_arrow) ? "token_fat_arrow" :		\
	(type == token_identifier) ? "token_identifier" :		\
	(type == token_string)    ? "token_string" :			\
	(type == token_i32)       ? "token_i32" :				\
	(type == token_i64)       ? "token_i64" :				\
	(type == token_f32)       ? "token_f32" :				\
	(type == token_f64)       ? "token_f64" :				\
	(type == token_and)       ? "token_and" :				\
	(type == token_class)     ? "token_class" :         \
    (type == token_struct)    ? "token_struct" :        \
	(type == token_else)      ? "token_else" :			\
	(type == token_false)     ? "token_false" :			\
	(type == token_for)       ? "token_for" :			\
	(type == token_fn)        ? "token_fn" :			\
	(type == token_if)        ? "token_if" :			\
	(type == token_none)      ? "token_none" :			\
	(type == token_or)        ? "token_or" :			\
	(type == token_return)    ? "token_return" :		\
	(type == token_super)     ? "token_super" :			\
	(type == token_self)      ? "token_self" :			\
	(type == token_true)      ? "token_true" :			\
	(type == token_var)       ? "token_var" :			\
	(type == token_while)     ? "token_while" :         \
    (type == token_loop)      ? "token_loop" :          \
	(type == token_break)     ? "token_break" :			\
	(type == token_continue)  ? "token_continue" :		\
	(type == token_match)     ? "token_match" :         \
    (type == token_enum)      ? "token_enum" :          \
	(type == token_error)     ? "token_error" :			\
	(type == token_eof)       ? "token_eof" :		    \
	"unknown"
// (type == token_print)     ? "token_print" :



/* display Token start parameter pointer offset length substring macro: used %.*s */
#define macro_token_get_lexeme(pointer, length)		\
	(int)length, pointer

/* Token type enum */
typedef enum TokenType {
	// Single-character tokens.
	token_left_paren, token_right_paren,			// ()
	token_left_bracket, token_right_bracket,		// []
	token_left_brace, token_right_brace,			// {}
	token_comma, token_dot, token_pipe,             // , . |
	token_colon, token_question, token_semicolon,   // : ? ;
    // - + * / %
    token_minus, token_plus, token_star, token_slash, token_mod,

    // += -= *= /= %=
    token_plus_assign, token_minus_assign,
    token_star_assign, token_slash_assign, token_mod_assign,

    // & &= | |= ^ ^= << <<= >> >>= ~ ~=
    token_bit_and, token_bit_and_assign,
    token_bit_or, token_bit_or_assign,
    token_bit_xor, token_bit_xor_assign,
    token_shl, token_shl_assign,
    token_shr, token_shr_assign,
    token_bit_not, token_bit_not_assign,

	// One or two character tokens.
	token_not, token_neq,					        // ! !=
	token_assign, token_eq,			                // = ==
	token_gt, token_egt,				            // > >=
	token_le, token_let,					        // < <=
	// -> (arrow), => (fat arrow)
	token_arrow, token_fat_arrow, token_underscore, token_layer,    // -> => _ ::
	// Literals.
	token_identifier, token_string,
	token_i32, token_i64, token_f32, token_f64,
    token_type,
	// string interpolation: {}

	// Keywords.
#ifndef deprecated_print_keyword
    token_print,
#endif

	token_and, token_class, token_struct, token_else, token_false,
	token_for, token_fn, token_if, token_none, token_or,
	token_return, token_super, token_self,
	token_true, token_var, token_while, token_break, token_continue,
	token_match, token_loop, token_enum,
	token_error, token_eof
} TokenType;

/* Token struct */
typedef struct Token {
	const char* start;
	int length;
	line_t line;
	TokenType type;
    struct Token *next;
} Token;

Token make_token(TokenType type, const char* start, int length, line_t line);
Token* new_token(VirtualMachine*vm, const char* start, int length, line_t line, TokenType type);
Token* eof_token(VirtualMachine *vm, line_t line);
void free_token(VirtualMachine *vm, Token* token);
void reset_token(Token* token);
bool equal_token(Token* left, Token* right);
bool equal_tkn(Token* left, const char* name);
void print_token(Token* token);


typedef struct TokenList {
    Token* head;
    Token* tail;
} TokenList;

TokenList* new_token_list(VirtualMachine* vm);
void list_add_token(TokenList* list, Token* token);
void list_extend_token(TokenList* to, TokenList* from, VirtualMachine* vm) ;
void print_tokens(TokenList* list);
void free_tokens(TokenList* list, VirtualMachine* vm);
Token* first_token(TokenList* list);
Token* last_token(TokenList* list);
Token* nth_token(TokenList* list, size_t n);

void tests();
#endif //JOKER_TOKEN_H
