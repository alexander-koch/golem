# Bubblesort algorithm in Golem.
# Copyright (c) Alexander Koch 2015
using core

# Bubblesort algorithm
func bubblesort(mut list: int[]) -> int[] {

	# Nested helper function
	func swap(i: int, j: int) {
		let temp = list[i]
		list[i] := list[j]
		list[j] := temp
	}

	# Actual algorithm
	let len = list.length()
	let mut swapped = true
	while swapped {
		swapped := false

		let mut k = 0
		while k < len-1 {
			if list[k] > list[k+1] {
				swap(k, k+1)
				swapped := true
			}
			k := k + 1
		}
	}

	return list
}

let t0 = clock()
let arr = [1,5,3,4,2,8,2,4,19,10,17,24,47,18,26]
println(bubblesort(arr))
let elapsed = clock() - t0
println("Elapsed: $elapsed")
