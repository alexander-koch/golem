# Example of nested functions
# Expected output:
# 2 + 3 + 2 + 4 + 5 + 5 + 6 = 27
using core

let x = 2
func func1(in:int) -> void {
	let y = 3 + in
	func func2(in1:int) -> void {
		let z = 4 + in1
		func func3(in2:int) -> void {
			let w = 5 + in2
			let sum = x + y + z + w
			println(sum)
		}
		func3(6)
	}
	func2(5)
}

func1(2)
