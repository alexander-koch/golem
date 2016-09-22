# Benchmark: Method calls and states.
# Copyright (c) Alexander Koch 2016
using core

type Toggle(_state: bool) {
	let mut state = _state

	func value() -> bool {
		return state
	}

	func activate() -> bool {
		state := !state
		return state
	}
}

func main() {
	let t0 = clock()
	let mut val = true
	let toggle = Toggle(val)

	let mut i = 0
	while i < 100000 {
		val := toggle.activate()
		i := i + 1
	}

    println(val)
	let elapsed = clock() - t0
	println("Elapsed: $elapsed")
}

main()
