/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifdef __GNUC__
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "hymn_os.h"

#ifdef _MSC_VER
#define HYMN_POPEN_SUPPORTED
#define POPEN _popen
#define PCLOSE _pclose
#elif _POSIX_C_SOURCE >= 2
#define HYMN_POPEN_SUPPORTED
#define POPEN popen
#define PCLOSE pclose
#endif

static HymnValue os_clock(Hymn *H, int count, HymnValue *arguments) {
    (void)H;
    (void)count;
    (void)arguments;
    return hymn_new_float((HymnFloat)((float)clock() / (float)CLOCKS_PER_SEC));
}

static HymnValue os_env(Hymn *H, int count, HymnValue *arguments) {
    if (count < 1) {
        return hymn_new_exception(H, "missing environment variable");
    }
    HymnValue value = arguments[0];
    if (!hymn_is_string(value)) {
        return hymn_new_exception(H, "environment variable must be a string");
    }
    HymnString *name = hymn_as_string(value);
    char *variable = getenv(name);
    if (variable == NULL) {
        return hymn_new_none();
    }
    HymnObjectString *string = hymn_new_intern_string(H, variable);
    return hymn_new_string_value(string);
}

static HymnValue os_system(Hymn *H, int count, HymnValue *arguments) {
#ifdef HYMN_POPEN_SUPPORTED
    if (count < 1) {
        return hymn_new_exception(H, "missing command");
    }
    HymnValue value = arguments[0];
    if (!hymn_is_string(value)) {
        return hymn_new_exception(H, "command must be a string");
    }
    HymnString *file = hymn_as_string(value);
    FILE *open = POPEN(file, "r");
    if (open == NULL) {
        return hymn_new_exception(H, "popen null pointer");
    }
    HymnString *string = hymn_new_string("");
    int ch;
    while ((ch = fgetc(open)) != EOF) {
        string = hymn_string_append_char(string, (char)ch);
    }
    PCLOSE(open);
    HymnObjectString *object = hymn_intern_string(H, string);
    return hymn_new_string_value(object);
#else
    (void)count;
    (void)arguments;
    return hymn_new_exception(H, "popen not supported");
#endif
}

static HymnValue os_exec(Hymn *H, int count, HymnValue *arguments) {
    if (count < 1) {
        return hymn_new_exception(H, "missing command");
    }
    HymnValue value = arguments[0];
    if (!hymn_is_string(value)) {
        return hymn_new_exception(H, "command must be a string");
    }
    HymnString *command = hymn_as_string(value);
    int result = system(command);
    return hymn_new_int(result);
}

static HymnValue os_popen(Hymn *H, int count, HymnValue *arguments) {
#ifdef HYMN_POPEN_SUPPORTED
    if (count < 2) {
        return hymn_new_exception(H, "missing path and mode");
    }
    HymnValue a = arguments[0];
    HymnValue b = arguments[1];
    if (!hymn_is_string(a) || !hymn_is_string(b)) {
        return hymn_new_exception(H, "path and mode must be strings");
    }
    HymnString *file = hymn_as_string(a);
    HymnString *mode = hymn_as_string(b);
    FILE *open = POPEN(file, mode);
    if (open == NULL) {
        return hymn_new_exception(H, "popen null pointer");
    }
    return hymn_new_pointer(open);
#else
    (void)count;
    (void)arguments;
    return hymn_new_exception(H, "popen not supported");
#endif
}

static HymnValue os_pclose(Hymn *H, int count, HymnValue *arguments) {
#ifdef HYMN_POPEN_SUPPORTED
    if (count < 1) {
        return hymn_new_exception(H, "missing pointer");
    }
    HymnValue value = arguments[0];
    if (!hymn_is_pointer(value)) {
        return hymn_new_exception(H, "argument not a pointer");
    }
    FILE *open = (FILE *)hymn_as_pointer(value);
    if (open == NULL) {
        return hymn_new_int(0);
    }
    int code = PCLOSE(open);
    return hymn_new_int(code);
#else
    (void)count;
    (void)arguments;
    return hymn_new_exception(H, "pclose not supported");
#endif
}

static HymnValue os_fopen(Hymn *H, int count, HymnValue *arguments) {
    if (count < 2) {
        return hymn_new_exception(H, "missing path and mode");
    }
    HymnValue a = arguments[0];
    HymnValue b = arguments[1];
    if (!hymn_is_string(a) || !hymn_is_string(b)) {
        return hymn_new_exception(H, "path and mode must be strings");
    }
    HymnString *path = hymn_as_string(a);
    HymnString *mode = hymn_as_string(b);
    FILE *open = fopen(path, mode);
    if (open == NULL) {
        return hymn_new_exception(H, "fopen null pointer");
    }
    return hymn_new_pointer(open);
}

static HymnValue os_fclose(Hymn *H, int count, HymnValue *arguments) {
    if (count < 1) {
        return hymn_new_exception(H, "missing pointer");
    }
    HymnValue value = arguments[0];
    if (!hymn_is_pointer(value)) {
        return hymn_new_exception(H, "argument must be a pointer");
    }
    FILE *open = (FILE *)hymn_as_pointer(value);
    if (open == NULL) {
        return hymn_new_int(0);
    }
    int code = fclose(open);
    return hymn_new_int(code);
}

static HymnValue os_fget(Hymn *H, int count, HymnValue *arguments) {
    if (count < 1) {
        return hymn_new_exception(H, "missing pointer");
    }
    HymnValue value = arguments[0];
    if (!hymn_is_pointer(value)) {
        return hymn_new_exception(H, "argument must be a pointer");
    }
    FILE *open = (FILE *)hymn_as_pointer(value);
    if (open == NULL) {
        return hymn_new_exception(H, "pointer was null");
    }
    HymnString *string = hymn_new_string("");
    int ch;
    while ((ch = fgetc(open)) != EOF) {
        string = hymn_string_append_char(string, (char)ch);
    }
    HymnObjectString *object = hymn_intern_string(H, string);
    return hymn_new_string_value(object);
}

void hymn_use_os(Hymn *H) {
    HymnTable *os = hymn_new_table();
    hymn_add_function_to_table(H, os, "clock", os_clock);
    hymn_add_function_to_table(H, os, "env", os_env);
    hymn_add_function_to_table(H, os, "popen", os_popen);
    hymn_add_function_to_table(H, os, "pclose", os_pclose);
    hymn_add_function_to_table(H, os, "fopen", os_fopen);
    hymn_add_function_to_table(H, os, "fclose", os_fclose);
    hymn_add_function_to_table(H, os, "fget", os_fget);
    hymn_add_function_to_table(H, os, "exec", os_exec);
    hymn_add_table(H, "os", os);

    hymn_add_function(H, "system", os_system);

    hymn_add(H, "stdout", hymn_new_int(0));
    hymn_add(H, "stderr", hymn_new_int(1));
}
