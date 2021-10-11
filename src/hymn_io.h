/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef HYMN_IO_H
#define HYMN_IO_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "hymn_util.h"

#ifdef __GNUC__
#include <dirent.h>
#include <linux/limits.h>
#include <unistd.h>
#define PATH_SEP '/'
#define PATH_SEP_STRING "/"
#elif _MSC_VER
#include <direct.h>
#define getcwd _getcwd
#define PATH_MAX FILENAME_MAX
#define PATH_SEP '\\'
#define PATH_SEP_STRING "\\"
#endif

struct FileList {
    int count;
    int capacity;
    HymnChar **files;
};

HymnChar *working_directory();
HymnChar *path_normalize(HymnChar *path);
HymnChar *path_absolute(HymnChar *path);
HymnChar *path_parent(HymnChar *path);
HymnChar *path_join(HymnChar *path, HymnChar *child);
size_t file_size(const char *path);
bool file_exists(const char *path);
HymnChar *cat(const char *path);
struct FileList directories(const char *path);
void delete_file_list(struct FileList *list);

#endif
