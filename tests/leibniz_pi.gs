# Calculate pi using the Leibniz series
# sum{k=0} (-1)^k / (2k + 1) = 1 - 1/3 + 1/5 - 1/7 + 1/9 - ... = pi/4

using core
using math

let n = 10000
let mut estimate = 4.0

let mut k = 1
while k < n {
    let mut sign = 4.0
    if (k % 2) = 1 {
        sign := -4.0
    }

    estimate := estimate + sign / (2.0 * k.to_f() + 1.0)
    println("pi = $estimate")

    k := k + 1
}
