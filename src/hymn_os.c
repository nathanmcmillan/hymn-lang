/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifdef __GNUC__
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "hymn.h"

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
    return hymn_new_float((double)clock() / (double)CLOCKS_PER_SEC);
}

static HymnValue os_env(Hymn *H, int count, HymnValue *arguments) {
    if (count < 1) {
        return hymn_new_none();
    }
    HymnValue value = arguments[0];
    if (!hymn_is_string(value)) {
        return hymn_new_none();
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
        return hymn_new_none();
    }
    HymnValue value = arguments[0];
    if (!hymn_is_string(value)) {
        return hymn_new_none();
    }
    HymnString *file = hymn_as_string(value);
    FILE *open = POPEN(file, "r");
    if (open == NULL) {
        return hymn_new_none();
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
    (void)H;
    (void)count;
    (void)arguments;
    return hymn_new_none();
#endif
}

static HymnValue os_popen(Hymn *H, int count, HymnValue *arguments) {
    (void)H;
#ifdef HYMN_POPEN_SUPPORTED
    if (count < 1) {
        return hymn_new_none();
    }
    HymnValue a = arguments[0];
    HymnValue b = arguments[1];
    if (!hymn_is_string(a) || !hymn_is_string(b)) {
        return hymn_new_none();
    }
    HymnString *file = hymn_as_string(a);
    HymnString *mode = hymn_as_string(b);
    FILE *open = POPEN(file, mode);
    if (open == NULL) {
        return hymn_new_none();
    }
    return hymn_new_pointer(open);
#else
    (void)count;
    (void)arguments;
    return hymn_new_none();
#endif
}

static HymnValue os_pclose(Hymn *H, int count, HymnValue *arguments) {
    (void)H;
#ifdef HYMN_POPEN_SUPPORTED
    if (count < 1) {
        return hymn_new_int(0);
    }
    HymnValue value = arguments[0];
    if (!hymn_is_pointer(value)) {
        return hymn_new_int(0);
    }
    FILE *open = (FILE *)hymn_as_pointer(value);
    if (open == NULL) {
        return hymn_new_int(0);
    }
    PCLOSE(open);
    return hymn_new_int(1);
#else
    (void)count;
    (void)arguments;
    return hymn_new_int(0);
#endif
}

static HymnValue os_fopen(Hymn *H, int count, HymnValue *arguments) {
    (void)H;
    if (count < 1) {
        return hymn_new_none();
    }
    HymnValue a = arguments[0];
    HymnValue b = arguments[1];
    if (!hymn_is_string(a) || !hymn_is_string(b)) {
        return hymn_new_none();
    }
    HymnString *path = hymn_as_string(a);
    HymnString *mode = hymn_as_string(b);
    FILE *open = fopen(path, mode);
    if (open == NULL) {
        return hymn_new_none();
    }
    return hymn_new_pointer(open);
}

static HymnValue os_fclose(Hymn *H, int count, HymnValue *arguments) {
    (void)H;
    if (count < 1) {
        return hymn_new_int(0);
    }
    HymnValue value = arguments[0];
    if (!hymn_is_pointer(value)) {
        return hymn_new_int(0);
    }
    FILE *open = (FILE *)hymn_as_pointer(value);
    if (open == NULL) {
        return hymn_new_int(0);
    }
    fclose(open);
    return hymn_new_int(1);
}

static HymnValue os_read(Hymn *H, int count, HymnValue *arguments) {
    if (count < 1) {
        return hymn_new_none();
    }
    HymnValue value = arguments[0];
    if (!hymn_is_pointer(value)) {
        return hymn_new_none();
    }
    FILE *open = (FILE *)hymn_as_pointer(value);
    if (open == NULL) {
        return hymn_new_none();
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
    hymn_add_function(H, "os.clock", os_clock);
    hymn_add_function(H, "os.env", os_env);
    hymn_add_function(H, "os.popen", os_popen);
    hymn_add_function(H, "os.pclose", os_pclose);
    hymn_add_function(H, "os.fopen", os_fopen);
    hymn_add_function(H, "os.fclose", os_fclose);
    hymn_add_function(H, "os.read", os_read);
    hymn_add_function(H, "system", os_system);
    hymn_add(H, "stdout", hymn_new_int(0));
    hymn_add(H, "stderr", hymn_new_int(1));
}
