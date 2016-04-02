# Fizzbuzz example
# Print 'Fizz' if divisible by 3
# Print 'Buzz' if divisible by 5
# If dibisible by 5 and 3, print 'FizzBuzz'
using core

let mut i = 1
while i <= 100 {
    let fizz = ((i % 3) = 0)
    let buzz = ((i % 5) = 0)

    if i % 15 = 0 {
        println("FizzBuzz")
    } else if(fizz) {
        println("Fizz")
    } else if(buzz) {
        println("Buzz")
    } else {
        println(i)
    }
    i := i + 1
}
