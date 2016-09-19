# Calculate pi using the Nilakantha series
# 3 + 4 / (2 * 3 * 4) - 4 / (4 * 5 * 6) + ...

using core
using math

let n = 150
let mut estimate = 3.0

let mut k = 0
while k < n {
    let mut sign = 1.0
    if (k % 2) = 1 {
        sign := -1.0
    }

    let n1 = (k + 1).to_f() * 2.0
    let n2 = n1+1.0
    let n3 = n1+2.0

    estimate := estimate + sign * 4.0 / (n1 * n2 * n3)
    println("pi = $estimate")

    k := k + 1
}
