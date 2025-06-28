//
// Created by Kilig on 2025/3/31.
//

#include <time.h>
#ifdef _WIN32
#include <windows.h>
#define sleep(x) Sleep((x) * 1000)
#else
#include <unistd.h>
#define sleep(x) usleep((x) * 1000000)
#endif

#include "string_.h"
#include "../include/time.h"


/* time block */
Value native_clock(VirtualMachine* vm, int arg_count, Value* args) {
    (void)vm;
    (void)arg_count;
    (void)args;

    return macro_val_from_f64(clock() / (double)CLOCKS_PER_SEC);
}

Value native_time(VirtualMachine* vm, int arg_count, Value* args) {
    (void)vm;
    (void)arg_count;
    (void)args;

    time_t t = time(NULL);
    return macro_val_from_f64((double)t);
}

Value native_sleep(VirtualMachine* vm, int arg_count, Value* args) {
    (void)vm;

    if (arg_count != 1) {
        panic("[ {PANIC} Native::sleep] Expected 1 argument, found %d", arg_count);
    }
    if (!macro_matches(args[0], VAL_I32)) {
        panic("[ {PANIC} Native::sleep] Expected argument to be an i32, found %s", macro_type_name(args[0]));
    }
    sleep(macro_as_i32(args[0]));
    return macro_val_null;
}

Value native_date(VirtualMachine* vm, int arg_count, Value* args) {
    (void)args;

    if (arg_count != 0) {
        panic("[ {PANIC} Native::date] Expected 0 arguments, found %d", arg_count);
    }

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char date_str[100];
    strftime(date_str, 100, "%Y-%m-%d %H:%M:%S", &tm);
    String* str = new_string(vm, date_str, (int32_t)strlen(date_str));
    return macro_val_from_obj(str);
}

Value native_now(VirtualMachine* vm, int arg_count, Value* args) {
    return native_date(vm, arg_count, args);
}
