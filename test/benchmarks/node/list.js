const process = require('process')

function reverse(a) {
  const size = a.length
  const max = Math.floor(size / 2)
  for (let i = 0; i < max; i++) {
    const n = size - i - 1
    const t = a[i]
    a[i] = a[n]
    a[n] = t
  }
}

const start = process.hrtime.bigint()

const N = 20000000
const list = []

for (let i = 0; i < N; i++) {
  list.push(i)
}

reverse(list)

const seconds = Number(process.hrtime.bigint() - start) / 1000000000
console.log('elapsed: ' + seconds + 's | ' + list[0])
