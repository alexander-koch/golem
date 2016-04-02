# Test for multidimensional arrays,
# which should not work by default.
# Expected: Multidimensional arrays are not permitted.
using core

let arr = [1,2,3,4,5]
println(arr)

let hugeArray = [arr, [1,2,3], [3]]
println(hugeArray)

let el = arr[4]
println(el)
