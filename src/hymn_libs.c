/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "hymn_libs.h"

#ifndef HYMN_NO_DYNAMIC_LIBS

#ifdef _MSC_VER
#include <windows.h>

typedef void (*HymnDynamicLib)(Hymn *H);

HymnString *hymn_use_dlib(Hymn *H, const char *path, const char *func) {
    HINSTANCE lib = LoadLibrary(path);
    if (lib != NULL) {
        HymnDynamicLib proc = (HymnDynamicLib)GetProcAddress(lib, func);
        if (proc != NULL) {
            proc(H);
            return NULL;
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
    return message;
}

#else
#include <dlfcn.h>

HymnString *hymn_use_dlib(Hymn *H, const char *path, const char *func) {
    void *lib = dlopen(path, RTLD_NOW);
    if (lib != NULL) {
        void *(*proc)(Hymn *);
        *(void **)(&proc) = dlsym(lib, func);
        if (proc != NULL) {
            proc(H);
            return NULL;
        }
    }

    HymnString *message = hymn_new_string(dlerror());
    if (lib != NULL) {
        dlclose(lib);
    }
    return message;
}

#endif

#endif
