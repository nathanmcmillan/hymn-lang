#!/usr/bin/env bash

HYMN=false
LUA=false
PYTHON=false
WREN=false
HYMN_NODE=false

if [ $# -eq 0 ]; then
  HYMN=true
  LUA=true
  PYTHON=true
  WREN=true
  HYMN_NODE=true
fi

while getopts "lwphn" option; do
  case "$option" in
    h) HYMN=true ;;
    l) LUA=true ;;
    p) PYTHON=true ;;
    w) WREN=true ;;
    n) HYMN_NODE=true ;;
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

if [ $WREN = true ]; then
  echo 'WREN'
  for file in test/benchmarks/wren/*.wren; do
    printf '> %s: %s\n' "$(basename "$file" .wren)" "$(wren "$file")"
  done
  echo $'----------------------------------------\n'
fi

if [ $HYMN_NODE = true ]; then
  echo 'HYMN NODE JS'
  for file in test/benchmarks/*.hm; do
    printf '> %s: %s\n' "$(basename "$file" .hm)" "$(js/cli.js "$file")"
  done
  echo $'----------------------------------------\n'
fi
