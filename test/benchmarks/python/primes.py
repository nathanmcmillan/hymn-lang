from time import process_time as clock

def is_prime(n):
    if n < 2 : return False
    for i in range(2, n):
        if n % i == 0 : return False
    return True

def main():
    start = clock()

    N = 30000
    primes = []

    for i in range(N):
        if is_prime(i):
            primes.append(i)

    seconds = clock() - start
    print("elapsed: " + str(seconds) + "s | " + str(primes[-1]))

main()
