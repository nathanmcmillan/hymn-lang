/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

const hymn = require('./hymn')

/* eslint-disable no-unused-vars */
function floor(H, count, args) {
  if (count < 1) {
    return hymn.newNone()
  }
  const arg = args[0]
  if (hymn.isFloat(arg)) {
    return hymn.newFloat(floor(arg.value))
  }
  return arg
}

module.exports = {
  floor: floor,
}
