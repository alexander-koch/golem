# Bubblesort algorithm in Golem.
# Copyright (c) Alexander Koch 2015
# To run this, use the try_catch branch
using core

# Bubblesort algorithm
func bubblesort(mut list: int[]) -> int[] {

	# Nested helper function
    @Exception
	func swap(i: int, j: int) {
		let temp = list[i]
		list[i] := list[j]
		list[j] := temp
	}

    try {
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
    } catch(e:Exception) {
        let msg = e.getMessage()
        println("Encountered exception: $msg")
        println("List remains unsorted")
    }

	return list
}

let arr = [1,5,3,4,2,8,2,4,19,10,17,24,47,18,26]
println(bubblesort(arr))
