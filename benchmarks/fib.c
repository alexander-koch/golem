#include <stdio.h>
#include <time.h>

int fib(int n) {
	if(n < 2) return n;
	return fib(n-2) + fib(n-1);
}

int main(int argc, char** argv) {
	
	clock_t t0 = clock();
	int i = 0;
	while(i < 5) {
		printf("%d\n", fib(28));
		i++;
	}
	
	double elapsed = (double)(clock() - t0) / CLOCKS_PER_SEC;
	printf("Elapsed: %f", elapsed);
	return 0;
}