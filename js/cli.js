#!/usr/bin/env node

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

const fs = require('fs')
const process = require('process')

const hymn = require('./hymn')

async function main() {
  if (process.argv.length <= 2) {
    console.info('Usage: hymn [-d] FILE')
    console.info('Interprets a Hymn script FILE.\n')
    console.info('  -d  Print byte')
  } else {
    let error = null
    if (process.argv.length >= 4) {
      const script = process.argv[3]
      const source = fs.readFileSync(script, { encoding: 'utf-8' })
      const vm = hymn.init()
      error = await hymn.debug(vm, script, source)
    } else {
      const script = process.argv[2]
      const source = fs.readFileSync(script, { encoding: 'utf-8' })
      const vm = hymn.init()
      error = await hymn.scriptInterpret(vm, script, source)
    }
    if (error !== null) {
      console.error(error)
    }
  }
}

main().catch((exception) => {
  console.error(exception)
})
