/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef LOG_H
#define LOG_H

#define LOG(m)                  \
    fprintf(stdout, "%s\n", m); \
    fflush(stdout)

#define ERROR(m)                                           \
    fprintf(stderr, "Line: %d, Error: %s\n", __LINE__, m); \
    fflush(stderr)

#define DEBUG(m)                                           \
    fprintf(stdout, "Line: %d, Debug: %s\n", __LINE__, m); \
    fflush(stdout)

#define DEBUG_CHAR(c)                                      \
    fprintf(stdout, "Line: %d, Debug: %c\n", __LINE__, c); \
    fflush(stdout)

#define DEBUG_INT(n)                                       \
    fprintf(stdout, "Line: %d, Debug: %d\n", __LINE__, n); \
    fflush(stdout)

#define DEBUG_USIZE(n)                                      \
    fprintf(stdout, "Line: %d, Debug: %zu\n", __LINE__, n); \
    fflush(stdout)

#endif
