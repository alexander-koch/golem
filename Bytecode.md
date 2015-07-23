# Bytecode Instruction Set

| Basic stack ops     | Description
|---                  |---
|iconst x             | pushes int onto the stack
|fconst x             | pushes float onto the stack
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

| Special             | Description
|---                  |---
|invoke x y           | invoke method x with y args, push return value (always!)
|jmp x                | unconditional jump
|jmpt x               | jump if true
|jmpf X               | jump if false

| Comparison          | Description
|---                  |---
|ieq                  | integer equal
|ine                  | integer not equal
|ilt                  | integer less than

# Example compilation

    let mut x = 5
    x = x + 5
    println(x)

This should compile to the following (unoptimized):

    Code:
      1: push_int, 5
      2: store_field, x, true
      3: get_field, x
      4: push_int, 5
      5: iadd
      6: store_field, x, false
      7: get_field, x
      8: invoke, println, 1

Optimized (not done by vm / interpreter):

    Code:
      1: push_int, 5
      2: push_int, 5
      3: add
      4: invoke, println, 1

Result of execution:

    >> 10
