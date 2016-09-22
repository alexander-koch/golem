# Benchmark: Calculate the 28th fibonacci number 5-times.
# Copyright (c) Alexander Koch 2015
using core

func fib(n: int) -> int {
	if n < 2 {
		return n
	}
	return fib(n-2) + fib(n-1)
}

let t0 = clock()
let mut i = 0
while i < 5 {
	println(fib(28))
	i := i + 1
}
let elapsed = clock() - t0
println("Elapsed: $elapsed")
