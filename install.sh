#!/bin/bash -e
cd "$(dirname "$0")"

if [[ -d ~/.hymn ]] && [[ ! -f ~/.hymn/hymn ]] ; then
  echo "Directory ~/.hymn already exists"
  exit 1
fi

echo "Compiling..."

gcc src/*.c -std=c11 -O3 -s -DNDEBUG -o hymn -lm || echo "Could not compile"

if [[ ! -f ./hymn ]]; then
  echo "Binary not found"
  exit 1
fi

echo "Success!"

mkdir -p ~/.hymn
mv hymn ~/.hymn

echo "Added binary to ~/.hymn"
echo "Please add ~/.hymn to your PATH"
