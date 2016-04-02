# Simple example to test classes
# Expected:
# 1. x := x + 1 -> 6
# 2. bar(x) -> 6*6 = 36
# 3. println(x) -> 36
# 4. println(cls.getX()) -> 36
using core

type Class(i:int) {
	@Getter
	let mut x = i

	func bar(y:int) {
		x := y * y
	}

	func foo() -> int {
		x := x + 1
		bar(x)
		println(x)
		return x
	}
}

let cls = Class(5)
cls.foo()
println(cls.getX())
