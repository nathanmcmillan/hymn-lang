from time import process_time as clock

start = clock()

N = 5000000
objects = []
result = 0
words = ["foo", "bar", "baz"]

for i in range(N):
    table = {}
    for word in words:
        table[word] = result
        result += i
    objects.append(table)

seconds = clock() - start
print("elapsed: " + str(seconds) + "s | " + str(objects[-1]['foo']))
