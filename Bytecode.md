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
|BEGIN_FUNC X Y       | creates a function with the name x and sets it to the current scope, y-parameters
|SCOPE_END            | pops a scope
|---                  |---
|JMPF X               | if top value is false, jump to x
|JMP x                | jump to x

# Example compilation

    let mut x = 5
    x = x + 5
    println(x)

should compile to

    PUSH_INT 5
    STORE_FIELD x true
    GET_FIELD x
    PUSH_INT 5
    ADD
    STORE_FIELD x true
    GET_FIELD x
    INVOKE println 1

should print out

    >> 10
