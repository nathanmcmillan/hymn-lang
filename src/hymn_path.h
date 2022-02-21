/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef HYMN_PATH_LIB_H
#define HYMN_PATH_LIB_H

#include "hymn.h"

struct HymnPathFileList {
    int count;
    int capacity;
    HymnString **files;
};

struct HymnPathFileList hymn_walk(const char *path, bool recursive);

void hymn_delete_file_list(struct HymnPathFileList *files);

#endif
