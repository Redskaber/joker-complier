//
// Created by Kilig on 2025/4/1.
//
#pragma once

#ifndef JOKER_REPL_H
#define JOKER_REPL_H
#define REPL_BUFFER_SIZE 1024
#define MAX_KEYWORDS 21
#define COLUMN_PER_LINE 4

#ifdef _WIN32
#include <windows.h>

// 兼容旧版Windows SDK
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#endif


int joker_entry(int argc, char* argv[]);

#endif //JOKER_REPL_H
