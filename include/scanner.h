//
// Created by Kilig on 2024/11/21.
//
#pragma once

#ifndef JOKER_SCANNER_H
#define JOKER_SCANNER_H
#include "common.h"
#include "token.h"

typedef struct Scanner {
    VirtualMachine *vm;
	TokenList* tokens;
	const char* start;
	const char* current;
	line_t line;
} Scanner;

void init_scanner(Scanner* self, VirtualMachine *vm, const char* source);
void free_scanner(Scanner* self);
Token* scan_token(Scanner* self);
void scan_tokens(Scanner* self);
#endif //JOKER_SCANNER_H
