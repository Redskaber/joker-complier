#include <stdio.h>
#include "../include/stdio.h"
#include "value.h"
#include "string_.h"
#include "error.h"

// 缓冲区安全写入宏：检查剩余空间并执行格式化写入
#define SAFE_WRITE(dest, pos, max, fmt, ...)                                    \
    do {                                                                        \
        int _n = snprintf((dest) + (pos), (max) - (pos), fmt, ##__VA_ARGS__);   \
        if (_n < 0 || (pos) + _n >= (max)) {                                    \
            panic("[ {PANIC} Native::print] Output buffer overflow");           \
            return macro_val_null;                                              \
        }                                                                       \
        pos += _n;                                                              \
    } while(0)


/* 转义字符处理表 */
static char escape_char(char c) {
    switch (c) {
        case 'n':  return '\n';
        case 't':  return '\t';
        case 'r':  return '\r';
        case '0':  return '\0';
        case '\\': return '\\';
        case '\'': return '\'';
        case '"':  return '\"';
        default:   return c;
    }
}

Value native_print(VirtualMachine* vm, int arg_count, Value* args) {
    (void)vm;

    if (arg_count < 1) {
        panic("[ {PANIC} Native::print] At least format string required");
        return macro_val_null;
    }

    const Value format_val = args[0];
    if (!macro_is_string(format_val)) {
        panic("[ {PANIC} Native::print] First argument must be a string (got %s)",
              macro_type_name(format_val));
        return macro_val_null;
    }

    const String* format_str = macro_as_string(format_val);
    const char* fmt = format_str->chars;
    const int fmt_len = format_str->length;

    char output[2048] = {0};
    int output_pos = 0;
    const int output_max = sizeof(output) - 1;
    int arg_index = 1;

    for (int fmt_pos = 0; fmt_pos < fmt_len && output_pos < output_max; fmt_pos++) {
        if (fmt[fmt_pos] == '\\') {
            fmt_pos++;
            if (fmt_pos >= fmt_len) {
                panic("[ {PANIC} Native::print] Incomplete escape sequence");
                return macro_val_null;
            }
            const char escaped = escape_char(fmt[fmt_pos]);
            SAFE_WRITE(output, output_pos, output_max, "%c", escaped);
            continue;
        }
        if (fmt[fmt_pos] != '%') {
            output[output_pos++] = fmt[fmt_pos];
            continue;
        }

        fmt_pos++;
        if (fmt_pos >= fmt_len) {
            panic("[ {PANIC} Native::print] Incomplete format specifier");
            return macro_val_null;
        }

        const char spec = fmt[fmt_pos];
        if (arg_index >= arg_count) {
            panic("[ {PANIC} Native::print] Missing argument for '%%%c' at position %d", spec, fmt_pos);
            return macro_val_null;
        }

        //-----------------------------
        // 参数类型检查与转换
        //-----------------------------
        const Value current_arg = args[arg_index];
        char elem_buf[256];
        snprintf_value(current_arg, elem_buf, sizeof(elem_buf));

        switch (spec) {
            case 'b': {
                if (!macro_is_bool(current_arg)) {
                    panic("[ {PANIC} Native::print] Expected boolean for '%%%c' (arg %d is %s)",
                          spec, arg_index, macro_type_name(current_arg));
                    return macro_val_null;
                }
                SAFE_WRITE(output, output_pos, output_max, "%s", elem_buf);
                arg_index++;
                break;
            }
            case 'd': {
                if (!macro_is_i32(current_arg) && !macro_is_i64(current_arg)) {
                    panic("[ {PANIC} Native::print] Expected integer for '%%%c' (arg %d is %s)",
                          spec, arg_index, macro_type_name(current_arg));
                    return macro_val_null;
                }
                SAFE_WRITE(output, output_pos, output_max, "%s", elem_buf);
                arg_index++;
                break;
            }
            case 'f': {
                if (!macro_is_f64(current_arg) && !macro_is_f32(current_arg)) {
                    panic("[ {PANIC} Native::print] Expected float for '%%%c' (arg %d is %s)",
                          spec, arg_index, macro_type_name(current_arg));
                    return macro_val_null;
                }
                SAFE_WRITE(output, output_pos, output_max, "%s", elem_buf);
                arg_index++;
                break;
            }
            case 's': {
                SAFE_WRITE(output, output_pos, output_max, "%s", elem_buf);
                arg_index++;
                break;
            }
            case '%': {
                SAFE_WRITE(output, output_pos, output_max, "%%");
                break;
            }
            default:
                panic("[ {PANIC} Native::print] Unsupported format specifier '%%%c'", spec);
                return macro_val_null;
        }
    }

    if (arg_index != arg_count) {
        panic("[ {PANIC} Native::print] Too many arguments (%d unused)",
              arg_count - arg_index);
        return macro_val_null;
    }

    printf("%s", output);
    return macro_val_null;
}

Value native_println(VirtualMachine* vm, int arg_count, Value* args) {
    const Value ret = native_print(vm, arg_count, args);
    printf("\n");
    return ret;
}

