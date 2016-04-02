# Test: Modify a class field, which is an array.
# Result should be: [5,2,3]
using core

type Class() {
	@Getter
	let mut x = [1,2,3]

	func mod() -> void {
		x[0] := 5
	}
}

let cls = Class()
cls.mod()
println(cls.getX())
