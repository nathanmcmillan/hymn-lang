/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef HYMN_STRING_LIB_H
#define HYMN_STRING_LIB_H

#include "hymn.h"

bool hymn_string_ends_with(HymnString *string, const char *using);
bool hymn_string_contains(HymnString *string, const char *using);

#endif
