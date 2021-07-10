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

shopt -s globstar

function fun {
  file="$1"
  header="$(head -n 1 "$file")"
  if [ "$header" != "$check" ]; then
    echo "$file"
    echo "$mozilla" | cat - "$file" > "$temp"
    mv "$temp" "$file"
  fi
}

mozilla='/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */
'

check='/* This Source Code Form is subject to the terms of the Mozilla Public'

for file in "src/"*".c"; do fun "$file"; done

for file in "src/"*".h"; do fun "$file"; done

mozilla='# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
'

check='# This Source Code Form is subject to the terms of the Mozilla Public'

for file in "src/"*".hm"; do fun "$file"; done
