#!/usr/bin/env node

/* Any copyright is dedicated to the Public Domain.
 * https://creativecommons.org/publicdomain/zero/1.0/ */

const vm = require('vm')
const fs = require('fs')
const path = require('path')
const process = require('process')

process.chdir(__dirname)

new vm.Script(fs.readFileSync('hymn.js')).runInThisContext()

const scripts = path.join('..', 'test', 'scripts')

let success = 0
let fail = 0
let count = 0

let out = ''

function parseExpected(source) {
  let expected = ''
  const size = source.length
  for (let pos = 0; pos < size; pos++) {
    let c = source[pos]
    if (c === '#' && pos + 1 < size && source[pos + 1] === ' ') {
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
    }
    break
  }
  return expected
}

function testSource(file) {
  file = path.join(scripts, file)
  const source = fs.readFileSync(file, { encoding: 'utf-8' })
  const expected = parseExpected(source)
  const hymn = new Hymn()
  hymn.print = (text) => {
    out += text + '\n'
  }
  out = ''
  let error = null
  try {
    error = hymnInterpret(hymn, source)
  } catch (exception) {
    error = exception
  }
  if (error) {
    return error
  }
  if (out !== expected) {
    return 'Expected: <<<\n' + expected + '>>> but was: <<<\n' + out + '>>>'
  }
  return null
}

function testArithmetic() {
  return testSource('arithmetic.hm')
}

function doTest(test) {
  count++
  console.log('    ', test)
  const result = test()
  if (result) {
    fail++
  } else {
    success++
  }
  return result
}

function testHymn() {
  const tests = [testArithmetic]
  for (const test of tests) {
    const result = doTest(test)
    if (result) return result
  }
  return null
}

function testSet(set) {
  console.log(set)
  const result = set()
  if (result) {
    console.error('Error:', result)
  }
}

console.log()
testSet(testHymn)
console.log(`Success: ${success}, Failed: ${fail}, Total: ${count}`)
console.log()
