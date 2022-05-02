/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef HYMN_LIBS_H
#define HYMN_LIBS_H

#include "hymn.h"

// #define HYMN_NO_LIBS
// #define HYMN_NO_DYNAMIC_LIBS

void hymn_use_os(Hymn *hymn);
void hymn_use_io(Hymn *hymn);
void hymn_use_path(Hymn *hymn);
void hymn_use_math(Hymn *hymn);
void hymn_use_json(Hymn *hymn);
void hymn_use_string(Hymn *hymn);
void hymn_use_pattern(Hymn *hymn);

#ifdef HYMN_NO_LIBS
#define hymn_use_libs(hymn) \
    do {                    \
    } while (0)
#else
#define hymn_use_libs(hymn) \
    hymn_use_os(hymn);      \
    hymn_use_io(hymn);      \
    hymn_use_path(hymn);    \
    hymn_use_math(hymn);    \
    hymn_use_json(hymn);    \
    hymn_use_string(hymn);  \
    hymn_use_pattern(hymn)
#endif

#ifndef HYMN_NO_DYNAMIC_LIBS
HymnString *hymn_use_dlib(Hymn *hymn, const char *path, const char *func);
#endif

#endif
