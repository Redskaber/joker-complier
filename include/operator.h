//
// Created by Kilig on 2024/11/21.
//
#pragma once

#ifndef JOKER_OPERATOR_H
#define JOKER_OPERATOR_H
typedef enum Operator {
    // binary operators
	ADD,                        // +
	SUB,                        // -
	MUL,                        // *
	DIV,                        // /
	MOD,                        // %
	POW,                        // ^
	EQ,                         // ==
	NEQ,                        // !=
	GT,                         // >
	LT,                         // <
	GTE,                        // >=
	LTE,                        // <=
	AND,                        // &&
	OR,                         // ||
	XOR,                        // ^|
    NEG,                        // -
	ASSIGN,                     // =
    SHL,                        // <<
    SHR,                        // >>
	BIT_AND,                    // &
	BIT_OR,                     // |
	BIT_XOR,                    // ^
    BINARY_COUNT,               // log binary count

    // unary operators
    NOT,                        // !
	BIT_NOT,                    // ~
    OPERATOR_COUNT              // unary count = OPERATOR_COUNT - BINARY_COUNT - 1
} Operator;

#define UNARY_COUNT                  (OPERATOR_COUNT - BINARY_COUNT - 1)
#define MACRO_IN_BINARY_OP(operator) (operator < BINARY_COUNT)
#define MACRO_IN_UNARY_OP(operator)  (operator > BINARY_COUNT && operator < OPERATOR_COUNT)
#define MACRO_BINARY_INDEX(operator) (operator)
#define MACRO_UNARY_INDEX(operator)  (operator - NOT)


#define macro_ops_to_string(op) \
    ((op) == ADD? "+" : \
     (op) == SUB? "-" : \
     (op) == MUL? "*" : \
     (op) == DIV? "/" : \
     (op) == MOD? "%" : \
     (op) == POW? "^" : \
     (op) == EQ ? "==" : \
     (op) == NEQ? "!=" : \
     (op) == GT ? ">" : \
     (op) == LT ? "<" : \
     (op) == GTE? ">=" : \
     (op) == LTE? "<=" : \
     (op) == AND? "&&" : \
     (op) == OR ? "||" : \
     (op) == XOR? "^|" : \
     (op) == SHL? "<<" : \
     (op) == SHR? ">>" : \
     (op) == NOT? "!" : \
     (op) == ASSIGN? "=" : \
     (op) == BIT_AND? "&" : \
     (op) == BIT_OR ? "|" : \
     (op) == BIT_XOR? "^" : \
     (op) == BIT_NOT? "~" : \
     (op) == NEG ? "-" : \
     "unknown operator")
#endif //JOKER_OPERATOR_H
