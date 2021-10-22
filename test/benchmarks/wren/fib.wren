var fib
fib = Fn.new {|n|
  if (n < 2) {
    return n
  }
  return fib.call(n - 1) + fib.call(n - 2)
}

var start = System.clock

var sum = 0

for (i in 0...10) {
  sum = sum + fib.call(30)
}

System.print("elapsed: %(System.clock - start)s | %(sum)")
