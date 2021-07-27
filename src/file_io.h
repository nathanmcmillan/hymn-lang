/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef FILE_IO_H
#define FILE_IO_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef __GNUC__
#include <linux/limits.h>
#include <unistd.h>
#define PATH_SEP '/'
#elif _MSC_VER
#include <direct.h>
#define getcwd _getcwd
#define PATH_MAX FILENAME_MAX
#define PATH_SEP '\\'
#endif

#include "mem.h"
#include "pie.h"
#include "string_util.h"

String *working_directory();
String *path_normalize(String *path);
String *path_absolute(String *path);
String *path_parent(String *path);
String *path_join(String *path, String *child);
usize file_size(const char *path);
bool file_exists(const char *path);
String *cat(const char *path);

#endif
