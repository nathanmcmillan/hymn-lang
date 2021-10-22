/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef HYMN_LIBS_H
#define HYMN_LIBS_H

#include "hymn.h"

void hymn_use_os(Hymn *hymn);
void hymn_use_io(Hymn *hymn);
void hymn_use_math(Hymn *hymn);

#define hymn_use_libs(hymn) \
    hymn_use_os(hymn);      \
    hymn_use_io(hymn);      \
    hymn_use_math(hymn)

#endif
