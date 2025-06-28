//
// Created by Kilig on 2025/5/7.
//

#include <stdio.h>
#include <string.h>

#include "common.h"
#include "console.h"


void console_help(int argc, char **argv) {
    (void)argc;
    (void)argv;
    printf("Usage: joker [options] [script]\n");
    printf("Options:\n");
    printf("  -h, --help               Print this help message and exit.\n");
    printf("  -v, --version            Print the version number and exit.\n");
    printf("  -i, --interactive        Start an interactive shell.\n");
    printf("  -e, --eval <code>        Evaluate the given code.\n");
    printf("  -c, --compile <file>     Compile the given file.\n");
    printf("  -m, --match <option>     Match the given option.\n");
    printf("  -o, --output <file>      Specify the output file.\n");
    printf("  -s, --stdin              Read from stdin.\n");
    printf("  -t, --test <file>        Run the given file as a test.\n");
    printf("  -w, --watch <file>       Watch the given file.\n");
}
void console_version(int argc, char **argv) {
    (void)argc;
    (void)argv;
    printf("Joker %s\n", JOKER_VERSION);
    printf("Copyright (c) 2025 Kilig\n");
    printf("All rights reserved.\n");
}

JokerConsoleFn console_match(const char* option) {
    if (strcmp(option, "-H") == 0
        || strcmp(option, "--help") == 0) {
        return console_help;
    } else if (strcmp(option, "-V") == 0
        || strcmp(option, "--version") == 0) {
        return console_version;
    }
    return NULL;
}
