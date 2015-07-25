# Bytecode Instruction Set

| Basic stack ops     | Description
|---                  |---
|push                 | pushes a generic value on the stack
|pop                  | pop value from stack, remove

| Store               | Description
|---                  |---
|store x              | pops a value and stores it in field x
|load X               | pushes the value of field x onto the stack

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
|concat               | concatenate strings

| Special             | Description
|---                  |---
|invoke x y           | invoke method x with y args, push return value (always!)
|jmp x                | unconditional jump
|jmpt x               | jump if true
|jmpf X               | jump if false

| Comparison          | Description
|---                  |---
|ieq                  | integer equal
|streq                | string equal
|ine                  | integer not equal
|ilt                  | integer less than

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
      08: invoke, println, 1

Optimized (not done by vm / interpreter):

    Code:
      01: iconst, 5
      02: iconst, 5
      03: iadd
      04: invoke, println, 1

Result of execution:

    >> 10
