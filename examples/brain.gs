# Simple brainf interpreter
# Copyright (c) Alexander Koch 2015
using core
using io

# Define tape and an index
let mut tape = [0]
let mut idx = 0

# Process an input character
func process(c:char) -> void {
	if(c = "+") {
		tape[idx] := tape[idx] + 1
	} else if(c = "-") {
		tape[idx] := tape[idx] - 1
	} else if(c = ">") {
		idx := idx + 1

		# Allocate more tape if needed
		if(idx > tape.length() - 1) {
			tape := tape.cons(0)
		}
	} else if(c = "<") {
		idx := idx - 1
	} else if(c = ".") {
		print(tape[idx])
	} else {
		# Invalid operation
		# Note: Newline and carriage return will end up here
	}
}

# The interpreter
func eval(filename:char[]) -> void {
	let file = readFile(filename)
	let len = file.length()

	# Process the tape
	let mut i = 0
	while(i < len) {
		process(file.at(i))
		i := i + 1
	}
}

# Run the interpreter
eval("main.bf")
