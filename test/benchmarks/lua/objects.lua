start = os.clock()

N = 5000000
objects = {}
sum = 0
words = {"foo", "bar", "baz"}

for i = 1, N - 1 do
    object = {}
    for _, word in ipairs(words) do
      object[word] = sum
      sum = sum + i
    end
    objects[#objects] = object
end

seconds = os.clock() - start
print('elapsed: ' .. seconds .. 's | ' .. objects[#objects].foo)
