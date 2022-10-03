#!/usr/bin/env node

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

const fs = require('fs')
const path = require('path')
const process = require('process')

process.chdir(__dirname)

const hymn = require('./hymn')

const scripts = path.join('..', 'test', 'language')

let success = 0
let fail = 0
let count = 0

function find() {
  const files = []
  const dir = fs.readdirSync(scripts)
  for (const item of dir) {
    const file = path.join(scripts, item)
    const stat = fs.lstatSync(file)
    if (stat.isDirectory()) {
      files.push(...find(file))
    } else if (file.endsWith('.hm')) {
      files.push(file)
    }
  }
  return files
}

function indent(spaces, text) {
  const pad = ' '.repeat(spaces)
  const array = text.split('\n')
  for (let i = 0; i < array.length; i++) {
    array[i] = pad + array[i]
  }
  return array.join('\n')
}

function parse(source) {
  let expected = ''
  const size = source.length
  for (let pos = 0; pos < size; pos++) {
    let c = source[pos]
    if (c === '#' && pos + 1 < size) {
      if (source[pos + 1] === ' ') {
        pos += 2
        while (pos < size) {
          c = source[pos]
          expected += c
          if (c === '\n') {
            break
          }
          pos++
        }
        continue
      } else if (source[pos + 1] === '\n') {
        pos++
        expected += '\n'
        continue
      }
    }
    break
  }
  return expected.trim()
}

async function testFile(file) {
  count++
  const source = fs.readFileSync(file, { encoding: 'utf-8' })
  const expected = parse(source)
  if (expected === '') {
    return null
  }
  let out = ''
  const vm = hymn.newVM()
  vm.print = (text) => {
    out += text
  }
  vm.printError = (text) => {
    out += text
  }
  vm.printLine = (text) => {
    out += text + '\n'
  }
  let result = null
  const error = await hymn.interpretScript(vm, file, source).catch((exception) => {
    if (exception.stack) {
      return indent(4, exception.stack)
    }
    return indent(4, exception)
  })
  if (expected.startsWith('@exception')) {
    if (error === null) {
      result = indent(4, 'Expected an error') + '\n' + indent(4, 'But was:') + '\n' + indent(8, out)
    } else {
      const except = expected.substring(11)
      if (!error.startsWith(except)) {
        result = indent(4, 'Expected error:') + '\n' + indent(8, except) + '\n' + indent(4, 'But was:') + '\n' + indent(8, error)
      }
    }
  } else {
    out = out.trim()
    if (error) {
      return indent(4, error.trim())
    } else if (expected.startsWith('@starts')) {
      const start = expected.substring(8)
      if (!out.startsWith(start)) {
        result = indent(4, 'Expected:') + '\n' + indent(8, start) + '\n' + indent(4, 'But was:') + '\n' + indent(8, out)
      }
    } else if (out !== expected) {
      result = indent(4, 'Expected:') + '\n' + indent(8, expected) + '\n' + indent(4, 'But was:') + '\n' + indent(8, out)
    }
  }
  if (result) {
    fail++
  } else {
    success++
  }
  return result
}

async function main() {
  console.log()
  const start = process.hrtime.bigint()
  let tests = find()
  if (process.argv.length >= 3) {
    const filter = process.argv[2]
    tests = tests.filter((file) => file.includes(filter))
  }
  tests.sort()
  for (const test of tests) {
    const file = path.basename(test)
    console.info(file)
    const result = await testFile(test)
    if (result) {
      console.error(result)
    }
  }
  const end = process.hrtime.bigint()
  console.log()
  console.log(`Success: ${success}, Failed: ${fail}, Total: ${count}, Time: ${(end - start) / BigInt(1000000)} ms`)
  console.log()
}

main()
