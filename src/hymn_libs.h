/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef HYMN_LIBS_H
#define HYMN_LIBS_H

// #define HYMN_NO_LIBS

#ifdef HYMN_NO_LIBS
#define hymn_use_libs(H) \
    do {                 \
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

#define hymn_use_libs(H) \
    hymn_use_os(H);      \
    hymn_use_io(H);      \
    hymn_use_path(H);    \
    hymn_use_math(H);    \
    hymn_use_json(H);    \
    hymn_use_text(H);    \
    hymn_use_glob(H);    \
    hymn_use_pattern(H)
#endif

#endif
