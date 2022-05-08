const process = require('process')

function fib(n) {
  if (n < 2) return n
  return fib(n - 1) + fib(n - 2)
}

const start = process.hrtime.bigint()

let sum = 0

for (let i = 0; i < 10; i++) {
  sum += fib(30)
}

const seconds = Number(process.hrtime.bigint() - start) / 1000000000
console.log('elapsed: ' + seconds + 's | ' + sum)
