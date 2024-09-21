/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

function useIO(H) {
  require('./hymn_io').useIO(H)
}

function useOS(H) {
  require('./hymn_os').useOS(H)
}

function usePath(H) {
  require('./hymn_path').usePath(H)
}

function useMath(H) {
  require('./hymn_math').useMath(H)
}

function useText(H) {
  require('./hymn_text').useText(H)
}

function useLibs(H) {
  useIO(H)
  useOS(H)
  usePath(H)
  useMath(H)
  useText(H)
}

module.exports = {
  useIO: useIO,
  useOS: useOS,
  usePath: usePath,
  useMath: useMath,
  useText: useText,
  useLibs: useLibs,
}
