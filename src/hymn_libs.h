/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef HYMN_LIBS_H
#define HYMN_LIBS_H

// #define HYMN_NO_LIBS

#ifdef HYMN_NO_LIBS
#define hymn_use_libs(hymn) \
    do {                    \
    } while (0)
#else
#include "hymn_glob.h"
#include "hymn_io.h"
#include "hymn_json.h"
#include "hymn_math.h"
#include "hymn_os.h"
#include "hymn_path.h"
#include "hymn_pattern.h"
#include "hymn_text.h"

#define hymn_use_libs(hymn) \
    hymn_use_os(hymn);      \
    hymn_use_io(hymn);      \
    hymn_use_path(hymn);    \
    hymn_use_math(hymn);    \
    hymn_use_json(hymn);    \
    hymn_use_text(hymn);    \
    hymn_use_glob(hymn);    \
    hymn_use_pattern(hymn)
#endif

#endif
