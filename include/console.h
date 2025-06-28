//
// Created by Kilig on 2025/5/7.
//
#pragma once

#ifndef JOKER_CONSOLE_H
#define JOKER_CONSOLE_H
#include "common.h"

typedef struct JokerConsole {
    const char *name;
    JokerConsoleFn fn;
} JokerConsole;

void console_help(int argc, char **argv);
void console_version(int argc, char **argv);

JokerConsoleFn console_match(const char* option);


#endif //JOKER_CONSOLE_H
