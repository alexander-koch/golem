# Implementation of a 2D Point/Vector
# Copyright (c) Alexander Koch 2015
using core
using math

type Point(xp: float, yp: float) {
	@Getter
	let x = xp

	@Getter
	let y = yp

	func distanceTo(other: Point) -> float {
		let dx = x - other.getX()
    	let dy = y - other.getY()
    	return sqrt(dx * dx + dy * dy)
	}
}

let p1 = Point(10.0,10.0)
let p2 = Point(0.0,0.0)
let dist = p1.distanceTo(p2)
println(dist)
