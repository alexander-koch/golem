# Simple virtual machine implemented in golem
using core

# Instructions
let OP_HLT = 0
let OP_PUSH = 1
let OP_POP = 2
let OP_ADD = 3
let OP_PRNT = 4

let program = [OP_PUSH, 5, OP_PUSH, 3, OP_ADD, OP_PRNT, OP_HLT]

let mut stack = [0,0,0,0,0,0,0,0,0]
let mut sp = -1
let mut pc = 0

func push(val: int) {
	sp := sp + 1
	stack[sp] := val
}

func pop() -> int {
	let val = stack[sp]
	sp := sp - 1
	return val
}

func fetch(ip: int) -> int {
	return program[ip]
}

func execute(opcode: int) {
	if opcode = OP_PUSH {
		pc := pc + 1
		push(fetch(pc))
	} else if opcode = OP_ADD {
		let v1 = pop()
		let v0 = pop()
		push(v0 + v1)
	} else if opcode = OP_PRNT {
		println(pop())
	}
}

let mut opcode = fetch(pc)
while opcode != OP_HLT {
	execute(opcode)
	pc := pc + 1
	opcode := fetch(pc)
	println(stack)
}
