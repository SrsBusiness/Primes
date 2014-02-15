all:
	gcc primes.c -o primes -std=c99 -lm -g
clean:
	rm primes primes.txt
