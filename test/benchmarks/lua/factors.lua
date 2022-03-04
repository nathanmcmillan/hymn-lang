start = os.clock()

N = 50000000
factors = {}

for i = 1, N do
  if N % i == 0 then
    factors[#factors] = i
  end
end

seconds = os.clock() - start
print('elapsed: ' .. seconds .. 's | ' .. factors[#factors])
