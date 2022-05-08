function fib(n, a, b)
  if n == 0 then return a end
  return fib(n - 1, b, a + b)
end

start = os.clock()

N = 50000
sum = 0

for i = 1, N do
  local number = fib(1000, 0, 1)
  if number > 1 then
    sum = sum + 1
  end
end

seconds = os.clock() - start
print('elapsed: ' .. seconds .. 's | ' .. sum)
