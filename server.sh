#!/usr/bin/env bash

if [ -e /tmp/hymn-in ]; then
  echo 'server already running'
  exit 1
fi
mkfifo /tmp/hymn-in
sleep infinity > /tmp/hymn-in &
echo $! > /tmp/hymn-pid
./hymn -s < /tmp/hymn-in &
