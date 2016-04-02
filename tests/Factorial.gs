# Test using the factorial function (recursion test).
# Expected: 120
using core

func fact(n:int) -> int {
	if(n = 0) {
		return 1
	}
	return fact(n - 1) * n
}

let result = fact(5)
println(result)
