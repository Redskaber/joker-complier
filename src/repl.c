//
// Created by Kilig on 2025/4/1.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "common.h"
#include "string_.h"
#include "vm.h"

#include "repl.h"
#include "console.h"



static void repl(VirtualMachine* vm);
static void repl_desc(void);
static void repl_help(void);
static void repl_keywords(void);
static bool is_repl_command(const char* line);
static void clear_screen(void);

static void run_file(VirtualMachine* vm, const char* path);
static char* read_file(const char* path);


// -------------------------------
// 全局变量
// -------------------------------
static bool color_enabled = false;
// -------------------------------
// 颜色支持检测与初始化
// -------------------------------
#ifdef _WIN32
static void enable_vt_mode(void) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;

    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) return;

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}
#endif

static bool supports_color(void) {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);  // 获取标准输出句柄
    if (hOut == INVALID_HANDLE_VALUE) return false;

    DWORD dwMode = 0; // 获取当前控制台模式
    if (!GetConsoleMode(hOut, &dwMode)) return false;
    return (dwMode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) != 0; // 检查是否支持虚拟终端处理
#else
    // Unix/Linux 平台处理
    const char* term = getenv("TERM");
    return term != NULL && strcmp(term, "dumb") != 0;
#endif
}

// 颜色宏定义（兼容无颜色终端）
#define COLOR_RESET   (color_enabled ? "\033[0m"    : "")
#define COLOR_CYAN    (color_enabled ? "\033[1;36m" : "")
#define COLOR_YELLOW  (color_enabled ? "\033[1;33m" : "")
#define COLOR_GREEN   (color_enabled ? "\033[1;32m" : "")
#define COLOR_RED     (color_enabled ? "\033[1;31m" : "")
#define COLOR_PURPLE  (color_enabled ? "\033[1;35m" : "")

// 初始化颜色支持检测
static void init_color_support(void) {
#ifdef _WIN32
    enable_vt_mode(); // Windows 强制启用虚拟终端
#endif
    color_enabled = supports_color();
}

/*
* The main entry point of the virtual machine.
* It reads the bytecode from a file and executes it.
* If no file is specified, it enters a REPL (read-eval-print loop).
*/
static inline void repl_desc(void) {
    printf("%sWelcome to Joker REPL (%s)!%s\n", COLOR_CYAN, JOKER_VERSION, COLOR_RESET);
    printf("%sType 'help' for commands, 'quit' to exit.%s\n", COLOR_CYAN, COLOR_RESET);
}

static void repl_help(void) {
    printf("%sJoker REPL Commands:%s\n", COLOR_YELLOW, COLOR_RESET);
    printf("  help      - Show this help message\n");
    printf("  keywords  - List language keywords\n");
    printf("  clear     - Clear the screen\n");
    printf("  version   - Show interpreter version\n");
    printf("  quit      - Exit the REPL\n");
}

static void repl_keywords(void) {
    const char* keywords[] = {
            "and", "class", "struct", "else", "false",
            "for", "fn", "if", "none", "or",
            "return", "super", "self", "true", "var",
            "while", "break", "continue", "match", "loop",
            "enum"
    };
    const size_t keyword_count = sizeof(keywords) / sizeof(keywords[0]);

    printf("%sJoker Keywords:%s\n", COLOR_GREEN, COLOR_RESET);
    for (size_t i = 0; i < keyword_count; i++) {
        if (i % COLUMN_PER_LINE == 0) printf("\n");
        printf("%-12s", keywords[i]);
    }
    printf("\n%s", COLOR_RESET);
}

static void clear_screen(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

static bool is_repl_command(const char* line) {
    if (strcmp(line, "help") == 0) {
        repl_help();
        return true;
    } else if (strcmp(line, "keywords") == 0) {
        repl_keywords();
        return true;
    } else if (strcmp(line, "clear") == 0) {
        clear_screen();
        return true;
    } else if (strcmp(line, "version") == 0) {
        printf("Joker Version: %s\n", JOKER_VERSION);
        return true;
    }
    return false;
}

static void repl(VirtualMachine* vm) {
    init_color_support();
    char line[REPL_BUFFER_SIZE];

    repl_desc();
    while (true) {
        printf("%s>>>%s ", COLOR_PURPLE, COLOR_RESET);
        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }

        line[strcspn(line, "\n")] = '\0';

        // 检测输入过长
        if (strlen(line) == REPL_BUFFER_SIZE - 1 && line[REPL_BUFFER_SIZE - 2] != '\n') {
            printf("%sInput exceeds %d characters!%s\n",
                   COLOR_RED, REPL_BUFFER_SIZE - 1, COLOR_RESET);
            int c;
            while ((c = getchar()) != '\n' && c != EOF); // 清空输入缓冲区
            continue;
        }
        if (line[0] == '\0' || is_repl_command(line)) continue;
        if (strcmp(line, "quit") == 0) break;

        InterpretResult result = interpret(vm, line);
        switch (result) {
            case interpret_ok:
                break;
            case interpret_compile_error:
                printf("%s[Error] Compilation failed!%s\n", COLOR_RED, COLOR_RESET);
                break;
            case interpret_runtime_error: {
                printf("%s[Runtime Error]%s %s\n", COLOR_RED, COLOR_RESET, "");
                break;
            }
            default:
                printf("%s[Unknown Error]%s\n", COLOR_RED, COLOR_RESET);
        }
    }
    printf("%sBye!%s\n", COLOR_CYAN, COLOR_RESET);
}


/*
* Runs the bytecode in a file.
* If there is a compilation or runtime error, it exits with an appropriate status code.
* If there is no file specified, it enters a REPL.
* If there is a file specified, it reads the bytecode from the file and executes it.
*/
static void run_file(VirtualMachine* vm, const char* path) {
    char* source = read_file(path);
    InterpretResult result = interpret(vm, source);
    free(source);

    if (result == interpret_compile_error) exit(enum_compiler_error);
    if (result == interpret_runtime_error) exit(enum_runtime_error);
}

/*
* Reads a file into a buffer.
* If there is an error, it exits with an appropriate status code.
* Returns a pointer to the buffer.
*/
static char* read_file(const char* path) {
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "[main::read_file] Error %s:\n\tCould not open file '%s'\n",
                macro_code_to_string(enum_file_error), path);
        exit(enum_file_error);
    }

    fseek(file, 0L, SEEK_END);
    size_t file_size = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(file_size + 1);
    if (buffer == NULL) {
        fprintf(stderr, "[main::read_file] Error %s:\n\tCould not allocate memory for file '%s'\n",
                macro_code_to_string(enum_file_error), path);
        exit(enum_file_error);
    }

    size_t bytes_read = fread(buffer, 1, file_size, file);
    if (bytes_read != file_size) {
        fprintf(stderr, "[main::read_file] Error %s:\n\tCould not read file '%s'\n",
                macro_code_to_string(enum_file_error), path);
        exit(enum_file_error);
    }

    buffer[bytes_read] = '\0';		// null-terminate the string

    fclose(file);
    return buffer;
}

void console_repl(VirtualMachine* vm, int argc, char* argv[]) {
    JokerConsoleFn console_fn = console_match(argv[1]);
    if (console_fn != NULL) {
        console_fn(argc, argv);
    } else {
        run_file(vm, argv[1]);
    }
}

int joker_entry(int argc, char* argv[]) {
    VirtualMachine vm;
    init_virtual_machine(&vm);

    switch (argc) {
        case 1: repl(&vm); break;
        case 2: console_repl(&vm, argc, argv); break;
        default:
            fprintf(stderr, "Usage: joker-compiler-c [path]\n");
            exit(enum_invalid_arguments);
    }

    free_virtual_machine(&vm);
    return 0;
}





