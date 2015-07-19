# Bytecode Instruction Set

| Instruction         | Description
|---------------------|-------------
|PUSH_INT             | pushes int onto the stack
|PUSH_FLOAT X         | pushes float onto the stack
|PUSH_STRING X        | pushes string onto the stack
|---                  |---
|GET_FIELD X          | pushes the value of field x onto the stack
|---                  |---
|ADD                  | pops two values from the stack and adds them, pushes result
|SUB                  | pops two values from the stack and substracts them, pushes result
|MUL                  | pops two values from the stack and multiplies them, pushes result
|DIV                  | pops two values from the stack and divides them, pushes result
|MOD                  | pops two values from the stack and applies modulo to them, pushes result
|---                  |---
|EQUAL                | pops two values from the stack and tests a == b, pushes a boolean
|LESS                 | pops two values from the stack and tests a < b, pushes a boolean
|---                  |---
|INVOKE X Y           | invokes the method X on top of the stack with y params
|---                  |---
|STORE_FIELD X Y      | pops a value and stores it as field x, marks it as mutable if y is true
|---                  |---
|PUSH_SCOPE X Y       | creates a function with the name x and sets it to the current scope, y-parameters
|POP_SCOPE            | pops the scope
|---                  |---
|JMPF X               | if top value is false, jump to x
|JMP x                | jump to x

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
      5: add
      6: store_field, x, false
      7: get_field, x
      8: invoke, println, 1

Result of execution:

    >> 10
