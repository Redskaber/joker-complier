//
// Created by Kilig on 2024/11/21.
//
#pragma once

#ifndef JOKER_DEBUG_H
#define JOKER_DEBUG_H
#include "common.h"

void disassemble_chunk(Chunk* chunk, const char* name);
int disassemble_instruction(Chunk* chunk, int offset);
#endif //JOKER_DEBUG_H
