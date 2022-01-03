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

async function testFile(test, file) {
  count++
  const result = await test(file)
  if (result) {
    fail++
  } else {
    success++
  }
  return result
}

function find(base) {
  const files = []
  const dir = fs.readdirSync(base)
  for (const item of dir) {
    const file = path.join(base, item)
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

function parseExpected(source) {
  let expected = ''
  const size = source.length
  for (let pos = 0; pos < size; pos++) {
    let c = source[pos]
    if (c === '-' && pos + 2 < size && source[pos + 1] === '-') {
      if (source[pos + 2] === ' ') {
        pos += 3
        while (pos < size) {
          c = source[pos]
          expected += c
          if (c === '\n') {
            break
          }
          pos++
        }
        continue
      } else if (source[pos + 2] === '\n') {
        pos += 2
        expected += '\n'
        continue
      }
    }
    break
  }
  return expected.trim()
}

async function testSource(script) {
  const source = fs.readFileSync(script, { encoding: 'utf-8' })
  const expected = parseExpected(source)
  if (expected === '') {
    return null
  }
  let out = ''
  const vm = hymn.init()
  vm.print = (text) => {
    out += text + '\n'
  }
  let result = null
  const error = await hymn.scriptInterpret(vm, script, source).catch((exception) => {
    if (exception.stack) {
      return indent(4, exception.stack)
    }
    return indent(4, exception)
  })
  if (expected === '@Exception') {
    if (error === null) {
      result = indent(4, 'Expected an error.')
    }
  } else {
    out = out.trim()
    if (error) {
      return indent(4, error.trim())
    } else if (expected.startsWith('@Starts')) {
      const start = expected.substring(8)
      if (!out.startsWith(start)) {
        result = indent(4, 'Expected:') + '\n' + indent(8, start) + '\n' + indent(4, 'But was:') + '\n' + indent(8, out)
      }
    } else if (out !== expected) {
      result = indent(4, 'Expected:') + '\n' + indent(8, expected) + '\n' + indent(4, 'But was:') + '\n' + indent(8, out)
    }
  }
  return result
}

function hymnFiles() {
  let files = find(scripts)
  if (process.argv.length >= 3) {
    const filter = process.argv[2]
    files = files.filter((file) => file.includes(filter))
  }
  files.sort()
  return files
}

async function testHymn() {
  const tests = hymnFiles()
  for (const test of tests) {
    const file = path.basename(test)
    console.info(file)
    const result = await testFile(testSource, test)
    if (result) {
      console.error(result)
    }
  }
}

async function main() {
  console.log()
  const start = process.hrtime.bigint()
  await testHymn()
  const end = process.hrtime.bigint()
  console.log()
  console.log(`Success: ${success}, Failed: ${fail}, Total: ${count}, Time: ${(end - start) / BigInt(1000000)} ms`)
  console.log()
}

main()
