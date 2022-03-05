from time import process_time as clock

def fib(n):
    if n < 2:
        return n
    return fib(n - 1) + fib(n - 2)

def main():
    start = clock()

    result = 0

    for i in range(0, 10):
        result += fib(30)

    seconds = clock() - start
    print("elapsed: " + str(seconds) + "s | " + str(result))

main()
