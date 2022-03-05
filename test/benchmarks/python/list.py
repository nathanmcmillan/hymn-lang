from time import process_time as clock
from math import floor

def reverse_list(data):
    size = len(data)
    end = floor(size / 2)
    for i in range(end):
        n = len(data) - i - 1
        t = data[i]
        data[i] = data[n]
        data[n] = t

def main():
    start = clock()

    N = 20000000
    data = list(range(N))

    reverse_list(data)

    seconds = clock() - start
    print("elapsed: " + str(seconds) + "s | " + str(data[0]))

main()
