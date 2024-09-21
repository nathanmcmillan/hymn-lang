/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

const hymn = require('./hymn')

/* eslint-disable no-unused-vars */
function mathFloor(H, count, args) {
  if (count < 1) {
    return hymn.newNone()
  }
  const value = args[0]
  if (hymn.isFloat(value)) {
    return hymn.newFloat(Math.floor(value.value))
  }
  return value
}

function useMath(H) {
  const math = new hymn.HymnTable()
  hymn.addFunctionToTable(math, 'floor', mathFloor)
  hymn.addTable(H, 'math', math)
}

module.exports = {
  useMath: useMath,
}
