const process = require('process')

const start = process.hrtime.bigint()

const N = 5000000
const objects = []
let sum = 0
const words = ['foo', 'bar', 'baz']

for (let i = 0; i < N; i++) {
  const object = {}
  for (let w = 0; w < words.length; w++) {
    const word = words[w]
    object[word] = sum
    sum += i
  }
  objects.push(object)
}

const seconds = Number(process.hrtime.bigint() - start) / 1000000000
console.log('elapsed: ' + seconds + 's | ' + objects[objects.length - 1].foo)
