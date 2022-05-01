/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "hymn_libs.h"

#ifndef HYMN_NO_DYNAMIC_LIBS

#ifdef _MSC_VER
#include <windows.h>

HymnValue hymn_use_dlib(Hymn *H, const char *path, const char *func) {
    HINSTANCE lib = LoadLibrary(path);
    if (lib != NULL) {
        FARPROC proc = GetProcAddress(lib, func);
        if (proc != NULL) {
            // proc(H);
            return hymn_new_none();
        }
    }

    HymnString *message = NULL;

    int error = GetLastError();
    char buffer[128];

    if (FormatMessage(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM, 0, error, 0, buffer, sizeof(buffer), 0)) {
        message = hymn_new_string(buffer);
    } else {
        message = hymn_string_format("windows error: %d\n", error);
    }

    if (lib != NULL) {
        FreeLibrary(lib);
    }

    HymnObjectString *object = hymn_intern_string(H, message);
    return hymn_new_string_value(object);
}

#else
#include <dlfcn.h>

HymnValue hymn_use_dlib(Hymn *H, const char *path, const char *func) {
    void *lib = dlopen(path, RTLD_NOW);
    if (lib != NULL) {
        void *(*proc)(Hymn *);
        *(void **)(&proc) = dlsym(lib, func);
        if (proc != NULL) {
            proc(H);
            return hymn_new_none();
        }
    }

    HymnString *message = hymn_new_string(dlerror());

    if (lib != NULL) {
        dlclose(lib);
    }

    HymnObjectString *object = hymn_intern_string(H, message);
    return hymn_new_string_value(object);
}

#endif

#endif
