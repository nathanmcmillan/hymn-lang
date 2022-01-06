#!/usr/bin/env node

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

const fs = require('fs')
const process = require('process')

const hymn = require('./hymn')
const libs = require('./hymn_libs')

async function main() {
  if (process.argv.length <= 2) {
    console.info('Usage: hymn [-b] [-c] FILE')
    console.info('Interprets a Hymn script FILE.\n')
    console.info('  -b  Print compiled byte code')
    console.info('  -c  Run FILE as source code')
    return
  }

  const vm = hymn.newVM()
  libs.use(vm)

  let error = null

  if (process.argv.length >= 4) {
    if (process.argv[2] === '-b') {
      if (process.argv.length >= 5) {
        if (process.argv[3] === '-c') {
          error = await hymn.debug(vm, null, process.argv[4])
        } else {
          console.error('Unknown second argument:', process.argv[3])
        }
      } else {
        const script = process.argv[3]
        const source = fs.readFileSync(script, { encoding: 'utf-8' })
        error = await hymn.debug(vm, script, source)
      }
    } else if (process.argv[2] === '-c') {
      error = await hymn.interpret(vm, process.argv[3])
    } else {
      console.error('Unknown argument:', process.argv[2])
    }
  } else {
    const script = process.argv[2]
    const source = fs.readFileSync(script, { encoding: 'utf-8' })
    error = await hymn.interpretScript(vm, script, source)
  }

  if (error !== null) {
    console.error(error)
  }
}

main().catch((exception) => {
  console.error(exception)
})
