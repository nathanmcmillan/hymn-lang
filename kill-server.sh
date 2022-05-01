#!/usr/bin/env bash

if [ -f /tmp/hymn-pid ]; then
  kill -s SIGTERM $(cat /tmp/hymn-pid)
  rm /tmp/hymn-in
  rm /tmp/hymn-pid
else
  echo 'server not found'
fi
