//
// Created by Kilig on 2024/11/21.
//
#pragma once

#ifndef JOKER_ERROR_H
#define JOKER_ERROR_H
#include <stdnoreturn.h>

void warning(const char* format, ...);
noreturn void error(const char* format, ...);
noreturn void panic(const char* format, ...) ;
#endif //JOKER_ERROR_H
