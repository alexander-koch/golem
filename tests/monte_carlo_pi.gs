# Calculate pi using monte carlo simulation

using core
using math

type Point(px: float, py: float) {
    let x = px
    let y = py
    func isInsideUnitCirle() -> bool {
        return x * x + y * y <= 1.0
    }
}

func generatePoints(count: int) -> Point[] {
    let mut list = [::Point]
    let mut i = 0
    while i < count {
        list := list.cons(Point(prng() * 2.0 - 1.0, prng() * 2.0 - 1.0))
        i := i + 1
    }
    return list
}

# n: Number of iterations
# batch: Number of points ber sample
func computePi(n: int, batch: int) {
	let mut total = 0.0
	let mut count = 0.0

	let mut k = 0
	while k < n {
		let points = generatePoints(batch)
		let mut i = 0
		while i < points.length() {
			if points[i].isInsideUnitCirle() {
				count := count + 1.0
			}
			i := i + 1
		}

		total := total + batch.to_f()
		let ratio = count / total
		let estimate = ratio * 4.0
		println("pi = $estimate")

		k := k + 1
	}
}

computePi(500, 200)
