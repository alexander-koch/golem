# Golem Bytecode Set

Golem runs on an internal stack-based bytecode virtual machine (gvm).
The following instructions are currently supported.

# VM Benchmark / Speed

Currently there are two implementations for the bytecode vm.
One is optimized and uses NaN-Tagging (GVM).
The following table demonstrates the speed.
For comparison python is also used.

| Program | Basic VM  | GVM    | Python |
|---      |---        |---     |---     |
| fib.gs  | ~4.95s    | ~1.08s | ~0.75s |

# Bytecode Instruction Set

| Basic stack ops     | Description
|---                  |---
|push                 | pushes a generic value on the stack
|pop                  | pop value from stack, remove
|hlt                  | halts the program

| Store               | Description
|---                  |---
|store x              | pops a value and stores it in the local field x
|load x               | pushes the value of the local field x onto the stack
|gstore x             | global store at address x
|gload x              | global load at address x
|ldarg0               | loads current class (argument 0) from stack frame
|setarg0              | sets current class in stack frame

| Arithmetic          | Description
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
|iminus               | integer sign change to minus
|fadd                 | float addition
|fsub                 | float subtraction
|fmul                 | float multiplication
|fdiv                 | float division
|fminus               | float sign change to minus
|not                  | logical negation operation

| Special             | Description
|---                  |---
|syscall x, y         | invokes an internal known method named x, with y args, push return value
|invoke x y           | invoke method at address x with y args, push return value
|invokevirtual x,y    | invokes a virtual class method, at address x with y args, and saves a class in the stack frame
|reserve x            | reserves x memory for function calls, to keep values in VRAM
|ret                  | returns from function to last instruction pointer
|retvirtual           | returns from a virtual class function
|jmp x                | unconditional jump
|jmpf x               | jump if false
|arr x                | build an array with the top x elements
|str x                | build a string with the top x elements
|len                  | length of an array (or string)
|append               | appends two arrays
|cons                 | constructs a new value onto an array

| Comparison          | Description
|---                  |---
|beq                  | boolean equal
|ieq                  | integer equal
|feq                  | float equal
|ceq                  | character equal
|bne                  | boolean not equal
|ine                  | integer not equal
|fne                  | float not equal
|cne                  | character not equal
|lt                   | number less than
|gt                   | number greater than
|le                   | number less equal
|ge                   | number greater equal
|band                 | boolean and
|bor                  | boolean or

| Subscript           | Description
|---                  |---
|getsub               | get the sub-element of the value, expects index (first) and value on top of the stack
|setsub               | sets the sub-element of the value, same mechanism as above

| Upval               | Description
|---                  |---
|upval x, y           | gets a value of the upper scope x, at the address y
|upstore x,y          | sets the value of scope x, at the address y to value on top of the stack

| Class               | Description
|---                  |---
|class x              | creates a class with the top x elements
|setfield x           | pop value, stores it in a field of the class
|getfield x           | get value x in of the class fields

# Method calling convention

### Function calls

All argument are first pushed on the stack, from first to last.
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

Function arguments are accessed using a negative index.

### Virtual function calls

Virtual functions are methods of classes.
The zeroth argument is the class itself and can be modified by the instructions
ldarg0 and setarg0. On virtual return the class has to be reassigned to it's original location.

| Stack        | Address |
|---           |---      |
|Stack bottom  |   0x00  |
|...		   |      ...|
|Class<>       |       -7|
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
      08: syscall, println, 1

Optimized (not done by vm or compiler):

    Code:
      01: push, 5
      02: push, 5
      03: iadd
      04: syscall, println, 1

Result of execution:

    >> 10
