#!/usr/bin/env bash
set -v
gcc src/*.c -std=c11 -Wall -Wextra -Werror -pedantic -Wno-unused-function -g -o hymn -lm
