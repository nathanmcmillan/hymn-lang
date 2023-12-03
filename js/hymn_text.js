/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

const hymn = require('./hymn')

/* eslint-disable no-unused-vars */
function contains(H, count, args) {
  if (count >= 2) {
    const value = args[0]
    const contains = args[1]
    if (hymn.isString(value) && hymn.isString(contains)) {
      const result = value.value.includes(contains.value)
      return hymn.newBool(result)
    }
  }
  return hymn.newNone()
}

function useText(H) {
  const table = new hymn.HymnTable()
  hymn.addFunctionToTable(table, 'contains', contains)
  hymn.addTable(H, 'text', table)
}

module.exports = {
  useText: useText,
}
