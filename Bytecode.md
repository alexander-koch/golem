# Golem Bytecode Set

Golem runs on an internal stack-based bytecode virtual machine (gvm).
Bytecode is used because of the speed compared to simple Tree-walkers, a JIT is not
used because of the complexity.

# VM Benchmark / Speed

The current VM uses a technique called NaN-Tagging.
It stores all values in a IEEE-754 64-bit floating-point value (double).
Integers, double-floats, booleans, characters, etc. and pointers are mapped onto it.
This greatly improves the speed and the memory usage.
On 64-bit systems, you can use the whole 64-bit for a double, and 32-bits for an integer.

The following table demonstrates the speed.
For comparison the old VM implementation is used.

(Info: The old VM used structs as values)

| Program | Old VM    | VM     | VM (using computed gotos) | Python 2.7.2 | Ruby    | Lua
|---      |---        |---     |---                        |---           |---      |---
| fib.gs  | ~4.95s    | ~0.72s | ~0.51s                    | ~0.73s       | ~0.29s  | ~0.31s

# Bytecode Instruction Set

| Basic stack ops     | Description
|---                  |---
|hlt                  | halts the program
|push                 | pushes a generic value on the stack
|pop                  | pop value from stack, remove

| Store               | Description
|---                  |---
|store x              | pops a value and stores it in the local field x
|load x               | pushes the value of the local field x onto the stack
|gstore x             | global store at address x
|gload x              | global load at address x
|ldarg0               | loads current class (argument 0) from stack frame
|setarg0              | sets current class in stack frame

| Arithmetic(Integer) | Description
|---                  |---
|iadd                 | integer addition, (pop 2 operands, add, push result)
|isub                 | integer subtraction
|imul                 | integer multiplication
|idiv                 | integer division
|mod                  | integer modulo
|bit_l                | left bit shift
|bit_r                | right bit shift
|bit_and              | binary 'and' operation
|bit_or               | binary 'or' operation
|bit_xor              | binary 'xor' operation
|bit_not              | binary 'not' operation
|iminus               | integer sign change
|i2f                  | convert an integer to a float

| Arithmetic(Float)   | Description
|---                  |---
|fadd                 | float addition
|fsub                 | float subtraction
|fmul                 | float multiplication
|fdiv                 | float division
|fminus               | float sign change
|f2i                  | converts a float to an integer

| Boolean             | Description
|---                  |---
|not                  | logical negation operation
|b2i                  | boolean to integer

| Special             | Description
|---                  |---
|syscall x            | invokes an internal known method at internal-index x, pushes a return value (similar to asm int-instruction)
|invoke x,y           | invoke method at address x with y args, push return value
|invokevirtual x,y    | invokes a virtual class method, at address x with y args, and saves a class in the stack frame
|reserve x            | reserves x memory for function calls, to keep values in VRAM
|ret                  | returns from function to last instruction pointer
|retvirtual           | returns from a virtual class function
|jmp x                | unconditional jump
|jmpf x               | jump if false
|arr x                | build an array with the top x elements
|str x                | build a string with the top x elements
|ldlib x              | loads library a (Experimental)
|tostr                | converts value on top of the stack to a string

| Comparison          | Description
|---                  |---
|beq                  | boolean equal
|ieq                  | integer equal
|feq                  | float equal
|bne                  | boolean not equal
|ine                  | integer not equal
|fne                  | float not equal
|ilt                  | integer less than
|igt                  | integer greater than
|ile                  | integer less equal
|ige                  | integer greater equal
|flt                  | float less than
|fgt                  | float greater than
|fle                  | float less equal
|fge                  | float greater equal
|band                 | boolean and
|bor                  | boolean or

| Subscript           | Description
|---                  |---
|getsub               | get the sub-element of the value, expects index (first) and value on top of the stack
|setsub               | sets the sub-element of the value, same mechanism as above
|len                  | length of an array (or string)
|append               | appends two arrays
|cons                 | constructs a new value onto an array

| Upval               | Description
|---                  |---
|upval x,y            | gets a value of the upper scope x, at the address y
|upstore x,y          | sets the value of scope x, at the address y to value on top of the stack

| Class               | Description
|---                  |---
|class x              | creates a class with the top x elements
|setfield x           | pop value, stores it in a field of the class
|getfield x           | get value x in of the class fields

# Method calling convention

### Function calls

All argument are pushed on the stack, from first to last.
Then the number of arguments is pushed, followed by the current frame pointer and program counter.
The frame pointer becomes the stack pointer, the pc is assigned to the new address.

| Stack        | Address |
|---           |---      |
|Stack bottom  |   0x00  |
|...		   |      ...|
|Arg0		   |      ...|
|Arg1		   |      ...|
|Arg2		   |      ...|
|...		   |	   -4|
|NUM_ARGS	   |       -3|
|FP			   |       -2|
|PC			   |       -1|
|...		   |	    0|	<-- current position fp / sp
|...		   |       +1|
|...           |      ...|
| Stack top    |    0x200|

Function arguments are accessed using a negative index (e.g. load -4 loads the first argument).

### Virtual function calls

Virtual functions are methods of classes.
The zeroth argument is the class itself and can be modified by the instructions
ldarg0 and setarg0. On virtual return the class has to be reassigned to it's original location.

| Stack        | Address |
|---           |---      |
|Stack bottom  |   0x00  |
|...		   |      ...|
|Class<>       |       -7|  <-- (use setarg0 / ldarg0 to access)
|Arg0		   |       -6|
|Arg1		   |       -5|
|Arg2		   |       -4|
|NUM_ARGS	   |       -3|
|FP			   |       -2|
|PC			   |       -1|
|...		   |	    0|	<-- current position fp / sp
|...		   |       +1|
|...           |      ...|
| Stack top    |    0x200|

# Example compilation

```rust
    using core

let mut x = 5
x := x + 5
println(x)
```

This should compile to the following instructions (unoptimized):

    Code:
      01: push, 5
      02: gstore, 0
      03: gload, 0
      04: push, 5
      05: iadd
      06: gstore, 0
      07: gload, 0
      08: syscall, 1, 1

Optimized (not done by vm or compiler, maybe in future versions):

    Code:
      01: push, 5
      02: push, 5
      03: iadd
      04: syscall, 1, 1

Result of execution:

    >> 10
