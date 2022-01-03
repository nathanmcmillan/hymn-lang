#!/usr/bin/env bash

echo "C"
echo "-"
grep -cve '^\s*$' src/*.c src/*.h
echo -n "= total:"
grep -v '^\s*$' src/*.c src/*.h | wc -l

echo ""
echo "JS"
echo "--"
grep -cve '^\s*$' js/*.js
echo -n "= total:"
grep -v '^\s*$' js/*.js | wc -l
