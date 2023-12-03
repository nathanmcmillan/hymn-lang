/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

const fs = require('fs')

const hymn = require('./hymn')

/* eslint-disable no-unused-vars */
function read(H, count, args) {
  if (count < 0) return hymn.newNone()
  const value = args[0]
  if (!hymn.isString(value)) return hymn.newNone()
  const source = fs.readFileSync(value.value, { encoding: 'utf-8' })
  return hymn.newString(source)
}

function useIO(H) {
  const io = new hymn.HymnTable()
  hymn.addFunctionToTable(io, 'read', read)
  hymn.addTable(H, 'io', io)
}

module.exports = {
  useIO: useIO,
}
