# Bytecode Instruction Set

| Instruction         | Description
|---------------------|-------------
|PUSH_INT             | pushes int onto the stack
|PUSH_FLOAT X         | pushes float onto the stack
|PUSH_STRING X        | pushes string onto the stack
|PUSH_NULL            | pushes NULL onto the stack
|---                  |---
|GET_FIELD X          | pushes the value of field x onto the stack
|---                  |---
|ADD                  | pops two values from the stack and adds them
|SUB                  | pops two values from the stack and substracts them
|MUL                  | pops two values from the stack and multiplies them
|DIV                  | pops two values from the stack and divides them
|MOD                  | pops two values from the stack and applies modulo to them
|---                  |---
|INVOKE X             | invokes the method on top of the stack with x params, pops x-times of the stack, pushes  return value on the stack
|---                  |---
|STORE_FIELD X Y      | pops a value and stores it as field x, marks it as mutable if y is true
|---                  |---
|BEGIN_FUNC X Y       | creates a function with the name x and sets it to the current scope, y-parameters
|SCOPE_END            | pops a scope
|---                  |---
|IF                   | if top stack value is true, executes until scope_end is reached

# Example compilation

    func main(args:str[]) -> void {
        let mut x = 5
        x = x + 5
        println(x)
    }

should compile to

    BEGIN_FUNC "main" 1
    PUSH_STRING "args"

    PUSH_INT 5
    STORE_FIELD "x" 1
    GET_FIELD "x"
    PUSH_INT 5
    ADD
    STORE_FIELD "x" 1
    GET_FIELD "x"
    PUSH "println"
    INVOKE 1
    SCOPE_END

should print out

    >> 10
