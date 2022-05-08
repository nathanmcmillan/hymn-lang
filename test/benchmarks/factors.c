/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "hymn.h"
#include "hymn_libs.h"

#ifdef HYMN_BENCHMARK

HymnValue compute(Hymn *vm, int count, HymnValue *arguments) {
    (void)vm;
    (void)count;
    (void)arguments;

    HymnInt N = hymn_as_int(hymn_get(vm, "N"));
    HymnArray *factors = hymn_as_array(hymn_get(vm, "factors"));

    for (HymnInt i = 1; i <= N; i++) {
        if (N % i == 0) {
            hymn_array_push(factors, hymn_new_int(i));
        }
    }

    return hymn_new_none();
}

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    Hymn *hymn = new_hymn();
    hymn_use_libs(hymn);

    hymn_add_function(hymn, "compute", compute);

    char *error = hymn_do(hymn, "let start = os.clock() let N = 50000000 let factors = [] compute() let seconds = os.clock() - start echo \"elapsed: \" + seconds + \"s | \" + factors[-1]");

    if (error != NULL) {
        fprintf(stderr, "%s\n", error);
        fflush(stderr);
        free(error);
    }

    hymn_delete(hymn);

    return error != NULL ? 1 : 0;
}

#endif
