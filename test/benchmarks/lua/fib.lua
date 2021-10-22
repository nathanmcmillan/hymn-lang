local function fib(n)
  if n < 2 then
    return n
  else
    return fib(n - 1) + fib(n - 2)
  end
end

local start = os.clock()

local sum = 0

for i = 1, 10 do
  sum = sum + fib(30)
end

local seconds = os.clock() - start
print('elapsed: ' .. seconds .. 's | ' .. sum)
