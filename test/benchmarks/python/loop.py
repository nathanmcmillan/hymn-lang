from time import process_time as clock

start = clock()

N = 10000000
data = []

for i in range(N):
    data.append(i)

total = 0

for i in data:
    total += data[i]

seconds = clock() - start
print("elapsed: " + str(seconds) + "s | " + str(total))
