# Grammar test
using core

# Test 1: function testing

# Void type
func f() {
	println("f()V")
}

# Return type int
func g() -> int {
	return 5
}

# Return type float
func h(i:int) -> float {
	return i.to_f()
}

# Test 2: Variables

# Function assignment 1:
# x:int
let x = g()

# Float
let y = 0.0

# Bool
let z = true


# Function assignment 2:
# y2:float
let y2 = h(x)

# Float addition
let y3 = y + y2

# Call f
f()

# Print out all values
println(x)
println(y)
println(z)
println(y2)
println(y3)
