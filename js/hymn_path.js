/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

const fs = require('fs')
const path = require('path')

const hymn = require('./hymn')

function files(get) {
  const list = []
  const dir = fs.readdirSync(get)
  for (const item of dir) {
    const file = path.join(get, item)
    const stat = fs.lstatSync(file)
    if (stat.isDirectory()) {
      list.push(...files(file))
    } else if (file.endsWith('.hm')) {
      list.push(file)
    }
  }
  return list
}

/* eslint-disable no-unused-vars */
function walk(H, count, args) {
  let get = null
  if (count < 0) {
    get = '.'
  } else {
    const value = args[0]
    if (!hymn.isString(value)) return hymn.newNone()
    get = value.value
  }
  const list = files(get)
  const size = list.length
  const array = []
  for (let f = 0; f < size; f++) {
    array.push(hymn.newString(list[f]))
  }
  return hymn.newArrayValue(array)
}

function usePath(H) {
  const table = new hymn.HymnTable()
  hymn.addFunctionToTable(table, 'walk', walk)
  hymn.addTable(H, 'path', table)
}

module.exports = {
  usePath: usePath,
}
