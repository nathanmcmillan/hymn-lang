local start = os.clock()

local N = 10000000
local list = {}

for i = 1, N do
  list[i] = i - 1
end

local sum = 0

for i = 1, N do
  sum = sum + list[i]
end

local seconds = os.clock() - start
print('elapsed: ' .. seconds .. 's')
