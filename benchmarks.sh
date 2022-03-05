#!/usr/bin/env bash

HYMN=false
LUA=false
PYTHON=false
NODE=false

if [ $# -eq 0 ]; then
  HYMN=true
  LUA=true
  PYTHON=true
  NODE=true
fi

while getopts "hlpn" option; do
  case "$option" in
    h) HYMN=true ;;
    l) LUA=true ;;
    p) PYTHON=true ;;
    n) NODE=true ;;
    *) exit 1 ;;
  esac
done

echo

if [ $HYMN = true ]; then
  echo 'HYMN'
  for file in test/benchmarks/*.hm; do
    printf '> %s: %s\n' "$(basename "$file" .hm)" "$(./hymn "$file")"
  done
  echo $'----------------------------------------\n'
fi

if [ $LUA = true ]; then
  echo 'LUA'
  for file in test/benchmarks/lua/*.lua; do
    printf '> %s: %s\n' "$(basename "$file" .lua)" "$(lua "$file")"
  done
  echo $'----------------------------------------\n'
fi

if [ $PYTHON = true ]; then
  echo 'PYTHON'
  for file in test/benchmarks/python/*.py; do
    printf '> %s: %s\n' "$(basename "$file" .py)" "$(python3 "$file")"
  done
  echo $'----------------------------------------\n'
fi

if [ $NODE = true ]; then
  echo 'NODE'
  for file in test/benchmarks/node/*.js; do
    printf '> %s: %s\n' "$(basename "$file" .js)" "$(node "$file")"
  done
  echo $'----------------------------------------\n'
fi
