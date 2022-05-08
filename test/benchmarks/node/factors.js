const process = require('process')

const start = process.hrtime.bigint()

const N = 50000000
const factors = []

for (let i = 1; i <= N; i++) {
  if (N % i === 0) {
    factors.push(i)
  }
}

const seconds = Number(process.hrtime.bigint() - start) / 1000000000
console.log('elapsed: ' + seconds + 's | ' + factors[factors.length - 1])
