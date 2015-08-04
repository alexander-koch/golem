# Golem Bytecode

Golem runs on an internal stack-based bytecode virtual machine (gvm).
The following instructions are currently supported.

# Bytecode Instruction Set

| Basic stack ops     | Description
|---                  |---
|push                 | pushes a generic value on the stack
|pop                  | pop value from stack, remove

| Store               | Description
|---                  |---
|store x              | pops a value and stores it in the local field x
|load x               | pushes the value of the local field x onto the stack
|gstore x             | global store at address x
|gload x              | global load at address x

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
|bne                  | boolean not equal
|ine                  | integer not equal
|fne                  | float not equal
|strne                | string not equal
|lt                   | number less than
|gt                   | number greater than
|le                   | number less equal
|ge                   | number greater equal
|band                 | boolean and
|bor                  | boolean or

| Subscript           | Description
|---                  |---
|strsub               | string subscript / sub-element
|arrsub               | array subscript / sub-element

# Example compilation

    let mut x = 5
    x = x + 5
    println(x)

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
