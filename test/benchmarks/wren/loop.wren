var start = System.clock

var N = 10000000
var list = []

for (i in 0...N) list.add(i)

var sum = 0

for (i in list) sum = sum + list[i]

System.print("elapsed: %(System.clock - start)s | %(sum)")
