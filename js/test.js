#!/usr/bin/env node

/* Any copyright is dedicated to the Public Domain.
 * https://creativecommons.org/publicdomain/zero/1.0/ */

const fs = require('fs')
const path = require('path')
const process = require('process')

process.chdir(__dirname)

const hymn = require('./hymn')

const scripts = path.join('..', 'test', 'language')

let success = 0
let fail = 0
let count = 0

function testFile(test, file) {
  count++
  const result = test(file)
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

let out = ''

function parseExpected(source) {
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

function testSource(file) {
  const source = fs.readFileSync(file, { encoding: 'utf-8' })
  const expected = parseExpected(source)
  if (expected === '') {
    return null
  }
  const vm = hymn.init()
  vm.print = (text) => {
    out += text + '\n'
  }
  out = ''
  try {
    const error = hymn.scriptInterpret(vm, file, source)
    if (expected === '@Exception') {
      if (error === null) {
        return indent(4, 'Expected an error.')
      }
    } else {
      out = out.trim()
      if (error) {
        return indent(4, error.trim())
      } else if (expected.startsWith('@Starts')) {
        const start = expected.substring(8)
        if (!out.startsWith(start)) {
          return indent(4, 'Expected:') + '\n' + indent(8, start) + '\n' + indent(4, 'But was:') + '\n' + indent(8, out)
        }
      } else if (out !== expected) {
        return indent(4, 'Expected:') + '\n' + indent(8, expected) + '\n' + indent(4, 'But was:') + '\n' + indent(8, out)
      }
    }
  } catch (exception) {
    if (exception.stack) {
      return indent(4, exception.stack)
    }
    return indent(4, exception)
  }
  return null
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

function testHymn() {
  const tests = hymnFiles()
  for (const test of tests) {
    const file = path.basename(test)
    const result = testFile(testSource, test)
    if (result) {
      console.error('⨯ ' + file)
      console.error(result)
    } else {
      console.log('✓ ' + file)
    }
  }
  return null
}

console.log()
const start = process.hrtime.bigint()
testHymn()
const end = process.hrtime.bigint()
console.log()
console.log(`Success: ${success}, Failed: ${fail}, Total: ${count}, Time: ${(end - start) / BigInt(1000000)} ms`)
console.log()
