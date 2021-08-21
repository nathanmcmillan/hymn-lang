#!/usr/bin/env bash
set -euo pipefail

temp='temp'

if [ -f "$temp" ]; then
  echo 'Temp file already exists'
  exit 1
fi

function cleanup {
  [ -f "$temp" ] && rm "$temp"
}
trap cleanup EXIT

shopt -s nullglob
shopt -s globstar

function fun {
  file="$1"
  header="$(head -n 1 "$file")"

  [[ "$header" = *'This Source Code Form is subject to the terms of the Mozilla Public'* ]] && return
  [[ "$header" = *'Any copyright is dedicated to the Public Domain.'* ]] && return
  [[ "$header" = '#!'* ]] && return

  echo "$file"
  echo "$mozilla" | cat - "$file" > "$temp"
  mv "$temp" "$file"
}

mozilla='/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */
'

for file in "src/"*".c"; do fun "$file"; done
for file in "src/"*".h"; do fun "$file"; done
for file in "js/"*".js"; do fun "$file"; done

mozilla='# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
'

for file in "src/"*".hm"; do fun "$file"; done
