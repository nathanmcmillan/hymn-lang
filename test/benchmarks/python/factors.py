from time import process_time as clock

start = clock()

N = 50000000
factors = []

for i in range(1, N + 1):
    if N % i == 0:
        factors.append(i)

seconds = clock() - start
print("elapsed: " + str(seconds) + "s | " + str(factors[-1]))
