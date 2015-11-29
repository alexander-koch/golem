# Bubblesort algorithm in Golem.
# Copyright (c) Alexander Koch 2015
using core

# Bubblesort algorithm
func bubblesort(mut list:int[]) -> int[] {

	# Nested helper function
	func swap(i:int, j:int) -> void {
		let temp = list[i]
		list[i] := list[j]
		list[j] := temp
	}

	# Actual algorithm
	let len = list.length()
	let mut swapped = true
	while(swapped) {
		swapped := false

		let mut k = 0
		while(k < len-1) {
			if(list[k] > list[k+1])
			{
				swap(k, k+1)
				swapped := true
			}
			k := k + 1
		}
	}

	return list
}

let arr = [5, 6, 1, 2, 9, 14, 2, 15, 6, 7, 8, 97]
println(bubblesort(arr))
