//
// Created by Kilig on 2024/12/23.
//
#pragma once

#ifndef JOKER_MOD_H
#define JOKER_MOD_H

#include "common.h"
#include "object.h"
#include "hashmap.h"


typedef struct Mod {
    Object base;
    String* name;
    HashMap exports;
} Mod;


#endif //JOKER_MOD_H
