const process = require('process')

function isPrime(n) {
  if (n < 2) return false
  const minus = n - 1
  for (let i = 2; i < minus; i++) {
    if (n % i === 0) return false
  }
  return true
}

const start = process.hrtime.bigint()

const N = 30000
const primes = []

for (let i = 0; i < N; i++) {
  if (isPrime(i)) {
    primes.push(i)
  }
}

const seconds = Number(process.hrtime.bigint() - start) / 1000000000
console.log('elapsed: ' + seconds + 's | ' + primes[primes.length - 1])
