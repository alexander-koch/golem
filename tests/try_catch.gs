# Test No.3: Try-catch testing
using core

let x = [1,2,3]
let index = 6

@Exception
func test() {
    let z = x[index]
}

try {
    test()
	let y = x[index]
	println(y)
} catch(e:Exception) {
    let msg = e.getMessage()
	println("Encountered exception: $msg")
}
