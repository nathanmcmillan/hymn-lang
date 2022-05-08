#!/usr/bin/env bash
set -v
gcc test/*.c src/*.c -std=c11 -Wall -Wextra -Werror -pedantic -Wno-unused-function -g -DHYMN_TESTING -Isrc -o hymntest -lm
