var reverse = Fn.new {|a|
  var size = a.count
  var max = (size / 2).floor
  for (i in 0..max) {
    var n = size - i - 1
    var t = a[i]
    a[i] = a[n]
    a[n] = t
  }
}

var start = System.clock

var N = 20000000
var list = []

for (i in 0...N) list.add(i)

reverse.call(list)

System.print("elapsed: %(System.clock - start)s | %(list[0])")
