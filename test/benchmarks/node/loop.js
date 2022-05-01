const process = require('process')

const start = process.hrtime.bigint()

const N = 10000000
const list = []

for (let i = 0; i < N; i++) {
  list.push(i)
}

let sum = 0

for (let i = 0; i < N; i++) {
  sum += list[i]
}

const seconds = Number(process.hrtime.bigint() - start) / 1000000000
console.log('elapsed: ' + seconds + 's | ' + sum)
