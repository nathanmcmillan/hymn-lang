/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

const hymn = require('./hymn')

/* eslint-disable no-unused-vars */
function clock(H, count, args) {
  return hymn.newFloat(Date.now() / 1000.0)
}

function useOS(H) {
  const os = new hymn.HymnTable()
  hymn.addFunctionToTable(os, 'clock', clock)
  hymn.addTable(H, 'os', os)
}

module.exports = {
  useOS: useOS,
}
