local function reverse(a)
  local len = #a
  local max = math.floor(len / 2)
  for i = 1, max do
    local n = len + 1 - i
    local t = a[i]
    a[i] = a[n]
    a[n] = t
  end
end

local start = os.clock()

local N = 20000000
local list = {}

for i = 1, N do
  list[i] = i - 1
end

reverse(list)

local seconds = os.clock() - start
print('elapsed: ' .. seconds .. 's | ' .. list[1])
