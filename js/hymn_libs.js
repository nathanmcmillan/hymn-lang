/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

const hymn = require('./hymn')

function useOS(vm) {
  const os = require('./hymn_os')
  hymn.addFunction(vm, 'os_clock', os.clock)
}

function useMath(vm) {
  const math = require('./hymn_math')
  hymn.addFunction(vm, 'math_floor', math.floor)
}

function use(vm) {
  useOS(vm)
  useMath(vm)
}

module.exports = {
  useOS: useOS,
  useMath: useMath,
  use: use,
}
