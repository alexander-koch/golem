using core

let pi = 3.1415926

func circle(radius:float) -> float {
	let area = pi * radius * radius
	return area
}

let area = circle(5.0)
println(area)
