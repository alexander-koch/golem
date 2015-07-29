# Golem Bytecode

Golem runs on an internal stack-based bytecode virtual machine.
The following instructions are currently supported.

# Bytecode Instruction Set

| Basic stack ops     | Description
|---                  |---
|push                 | pushes a generic value on the stack
|pop                  | pop value from stack, remove

| Store               | Description
|---                  |---
|store x              | pops a value and stores it in field x
|load x               | pushes the value of field x onto the stack

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
|concat               | concatenate strings

| Special             | Description
|---                  |---
|syscall x, y         | invokes an internal known method named x, with y args, push return value
|invoke x y           | invoke method at address x with y args, push return value
|ret                  | returns from function to last instruction pointer
|jmp x                | unconditional jump
|jmpt x               | jump if true
|jmpf X               | jump if false

| Comparison          | Description
|---                  |---
|beq                  | boolean equal
|ieq                  | integer equal
|feq                  | float equal
|streq                | string equal
|ine                  | integer not equal
|ilt                  | integer less than
|igt                  | integer greater than
|band                 | boolean and
|bor                  | boolean or

# Example compilation

    let mut x = 5
    x = x + 5
    println(x)

This should compile to the following (unoptimized):

    Code:
      01: iconst, 5
      02: store, 0
      03: load, 0
      04: iconst, 5
      05: iadd
      06: store, 0
      07: load, 0
      08: syscall, println, 1

Optimized (not done by vm / interpreter):

    Code:
      01: iconst, 5
      02: iconst, 5
      03: iadd
      04: syscall, println, 1

Result of execution:

    >> 10
