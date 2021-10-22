#!/bin/bash

LUA=false
WREN=false
PYTHON=false
HYMN=false
HYMN_NODE=false

if [ $# -eq 0 ]; then
  LUA=true
  WREN=true
  PYTHON=true
  HYMN=true
  HYMN_NODE=true
fi

while getopts "lwphn" option; do
  case "$option" in
    l) LUA=true ;;
    w) WREN=true ;;
    p) PYTHON=true ;;
    h) HYMN=true ;;
    n) HYMN_NODE=true ;;
    *) exit 1 ;;
  esac
done

echo

if [ $LUA = true ]; then
  echo 'LUA'
  for file in test/benchmarks/lua/*.lua; do
    printf '> %s: %s\n' "$(basename "$file" .lua)" "$(lua "$file")"
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

if [ $PYTHON = true ]; then
  echo 'PYTHON'
  for file in test/benchmarks/python/*.py; do
    printf '> %s: %s\n' "$(basename "$file" .py)" "$(python3 "$file")"
  done
  echo $'----------------------------------------\n'
fi

if [ $HYMN = true ]; then
  echo 'HYMN'
  for file in test/benchmarks/*.hm; do
    printf '> %s: %s\n' "$(basename "$file" .hm)" "$(./hymn "$file")"
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
