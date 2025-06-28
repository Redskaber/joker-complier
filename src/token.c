//
// Created by Kilig on 2024/11/21.
//
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "error.h"
#include "memory.h"
#include "token.h"


Token make_token(TokenType type, const char* start, int length, line_t line) {
	Token token;
	token.type = type;
	token.start = start;
	token.length = length;
	token.line = line;
    token.next = NULL;
	return token;
}

/* Token */
Token* new_token(VirtualMachine*vm, const char* start, int length, line_t line, TokenType type) {
	Token* token = macro_allocate(vm, Token, 1);
	if (token == NULL) {
		panic("[ {PANIC} Token::create_token] Expected to allocate memory for token, Found NULL");
	}
	token->type = type;
	token->start = start;
	token->length = length;
	token->line = line;
    token->next = NULL;
	return token;
}

void free_token(VirtualMachine *vm, Token* token) {
	if (token != NULL) {
		macro_free(vm, Token, token);
	}
}

void print_token(Token* token) {
	if (token != NULL) {
		printf("Token(type: %s, lexeme: \"%.*s\", length: %d, line: %d)",
			macro_token_type_to_string(token->type),
			macro_token_get_lexeme(token->start, token->length),
			token->length,
			token->line
		);
	}
	else {
		printf("Token(NULL)");
	}
}

Token* eof_token(VirtualMachine *vm, line_t line) {
    return new_token(vm, "", 0, line, token_eof);
}

bool equal_token(Token* left, Token* right) {
    return left->length == right->length &&
           memcmp(left->start, right->start, left->length) == 0;
}

bool equal_tkn(Token* left, const char* name) {
    return strlen(name) == (size_t)left->length &&
          memcmp(left->start, name, left->length) == 0;
}

/* TokenList */
TokenList* new_token_list(VirtualMachine* vm) {
	TokenList* list = macro_allocate(vm, TokenList, 1);
	if (list == NULL) {
		panic("[ {PANIC} TokenList::new_token_list] Expected to allocate memory for token list, Found NULL");
	}
    list->head = NULL;
    list->tail = NULL;
	return list;
}

void free_tokens(TokenList* list, VirtualMachine* vm) {
    if (list != NULL) {
        Token* node = list->head;
        while (node != NULL) {
            Token* next = node->next;
            free_token(vm, node);
            node = next;
        }
        macro_free(vm, TokenList, list);
    }
}

void list_add_token(TokenList* list, Token* token) {
	if (list->head == NULL) {
		list->head = token;
		list->tail = list->head;
	}
	else {
        list->tail->next = token;
        list->tail = token;
	}
}

void list_extend_token(TokenList* to, TokenList* from, VirtualMachine* vm) {
    if (from == NULL) return;
    if (to == NULL) {
        panic("[ {PANIC} TokenList::list_extend_token] Expected to extend token list, Found NULL");
    }

    if (to->head == NULL) {
        to->head = from->head;
        to->tail = from->tail;
    } else {
        to->tail->next = from->head;
        to->tail = from->tail;
    }
    macro_free(vm, TokenList, from);
}

void print_tokens(TokenList* list) {
	printf("Tokens:\n");
    Token* node = list->head;
    while (node != NULL) {
		print_token(node);
		printf("\n");
		node = node->next;
	}
    printf("\n");
}

Token* first_token(TokenList* list) {
	if (list->head == NULL) {
		return NULL;
	}
	else {
		return list->head;
	}
}

Token* last_token(TokenList* list) {
	if (list->tail == NULL) {
		return NULL;
	}
	else {
		return list->tail;
	}
}

Token* nth_token(TokenList* list, size_t n) {
    Token* node = list->head;
    for (size_t i = 0; i < n; i++) {
        node = node->next;
        if (node == NULL) {
            return NULL;
        }
    }
    return node;
}
