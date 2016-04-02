using core

# TODO: consider adding this to the standard library
type string(data:char[]) {
	let chars = data
	let len = data.length()

    # Get a char at index
	func charAt(i:int) -> char {
		return chars[i]
	}

    # Compare two strings
	func equals(other:string) -> bool {
		let mut eq = true
		let mut i = 0
		while i < len {
			if other.charAt(i) != charAt(i) {
				eq := false
			}
			i := i + 1
		}
		return eq
	}
}

let x = string("Hello World!")
let y = string("Hello Korld!")
let result = x.equals(y)
println("The result is: $result")
