#!/usr/bin/env node

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

const fs = require('fs')
const process = require('process')

const hymn = require('./hymn')
const libs = require('./hymn_libs')

async function main() {
  const args = process.argv

  if (args.length <= 2) {
    console.info('usage: hymn [-b] [-c] FILE')
    console.info('interprets a hymn script FILE.\n')
    console.info('  -b  print compiled byte code')
    console.info('  -c  run FILE as source code')
    return
  }

  const vm = hymn.newVM()
  libs.use(vm)

  let error = null

  if (args.length >= 4) {
    if (args[2] === '-b') {
      if (args.length >= 5) {
        if (args[3] === '-c') {
          error = await hymn.debug(vm, null, args[4])
        } else {
          console.error('unknown second argument:', args[3])
        }
      } else {
        const script = args[3]
        const source = fs.readFileSync(script, { encoding: 'utf-8' })
        error = await hymn.debug(vm, script, source)
      }
    } else if (args[2] === '-c') {
      error = await hymn.interpret(vm, args[3])
    } else {
      console.error('unknown argument:', args[2])
    }
  } else {
    const script = args[2]
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
