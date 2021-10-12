#!/bin/bash

echo

echo 'LUA'
for file in test/benchmarks/lua/*.lua; do
  printf '> %s: %s\n' "$(basename "$file" .lua)" "$(lua "$file")"
done
echo $'----------------------------------------\n'

echo 'HYMN'
for file in test/benchmarks/*.hm; do
  printf '> %s: %s\n' "$(basename "$file" .hm)" "$(./hymn "$file")"
done
echo $'----------------------------------------\n'

echo 'HYMN JS'
for file in test/benchmarks/*.hm; do
  printf '> %s: %s\n' "$(basename "$file" .hm)" "$(js/cli.js "$file")"
done
echo $'----------------------------------------\n'
