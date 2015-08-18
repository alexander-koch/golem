# Project Golem-lang

A programming language written in C.

# Project aims

- Functional, object-oriented scripting / programming language
- Immutablitiy of everything
- Explicit mutable declaration
- Statically, strong typed
- Type inference => automatic deduction of data types
- Strings as arrays of characters, not a standalone type
- Systems language
- High-order functions, currying
- Fully functional fast bytecode vm or translation into x86 / x86_64 assembler machine code
- Garbage collected vm
- Easily readable and learnable
- FAST!!!

# TODO

#### Serious issues

- [ ] When trying to replace an **Upvalue**, wrong storage address is used => (Recognized, Working on it immediately)

#### Other

- [ ] Nested functions => (Nearly done.)
- [ ] Bytecode optimizations => (Working on it)
- [ ] Lambdas, Closures, Anonymous functions => (Prototype / Concept)
- [x] Implementing all (basic) operators for associated types in the vm / bytecodes => (Done.)
- [x] Finalize implementing functions => (Done.)
- [x] mark-sweep garbage collector for vm => (Done.)
- [x] Mutable parameters => (Done.)
- [x] Subscripts (Arrays / Strings) => (Done.)

#### Langugage specific


- AST optimization
- prefix operators
- Lambdas
- Typecasting > Done by std library

#### Implementation specific

- map / filter / curry methods
- performance and bytecode optimization

# Concept (final concept may change)

### Variables

Each variable is immutable at first. Variables are defined / declared using the 'let' keyword.
Example:
```rust
	let x = 5
```

Variables can be mutable, if they are declared using the 'mut' keyword.
```rust
	let mut x = 5
x = x + 1
```

### Functions

Functions are declared using the 'func' keyword.
The body is bound by egyptian style braces. You have to declare the parameters and the return type.
Parameters are always immutable, if the 'mut' mutability keyword is not set.
```ruby
	func main(arg0:char[]) -> void {
	# body
}
```

### Arrays

Arrays are defined using two brackets, in which the content is placed.
An array can only be of one datatype and can not be replaced of modified if it is immutable.
For character arrays, a string initializer can be used.

Integer array:
```rust
	let arr = [1,2,3,4,5]
let str = "Hello World" // Same as ["H", "E", "L", ...]
```

### Classes (Not implemented, Still concept)

Classes are defined using the structure below. The method `new` is the classes constructor.
All attributes are private.
To get a field, getters are used to maintain encapsulation.
```ruby
	import core

object Class {

	# Attributes
	let mut x = 0
	let mut y = 0
	let z = 0

	# Constructor
	func new(x: int, y: int, z: int) -> void {
		self.x = x
		self.y = y
		self.z = z
	}

	# Method 1
	func run() -> void {
		println("Running with x:", self.x, "and y:", self.y)
	}

	# More methods here [...]

	# Example for encapsulation
	func getX() -> int {
		return x
	}
}

func main() -> void {
	let cls = Class::new(7,5,2)
	cls.run()
	println(cls.getX())
}

main()
```

### Control Flow

Control flows are created by if statements or while loops. Style is egyptian.
Example:
```rust
	if(number == 5) {
	println("Your number is odd.")
} else if(number == 3) {
	println("Your number is odd and it's three.")
} else {
	println("Your number isn't 5 or 3.")
}
```
While loops:
```ruby
	while(number == 5) {
	number = number + 1
}
```

# Curently supported

### AST

- variable declaration (immutable/mutable)
- function declaration
- if statements
- while loop
- function calls
- library imports
- array declaration
- subscripts
- expressions with precedence
- classes

### Langugage-based

- variable and function declaration
- types int, char, float, bool, (void)
- function calls, recursion
- arrays
- system internal functions, println + getline
- fast bytecode vm
- compiler error reports

# Stuff / Internet sources

http://markmail.org/download.xqy?id=vhdi5dbzkrdbchxq&number=1
http://llvm.org/docs/LangRef.html
http://llvm.org/docs/doxygen/html/group__LLVMCCoreValueGeneral.html
https://github.com/TheThirdOne/llvm-tutorial-gitbook/blob/master/mutable.md
http://ducklang.org/designing-a-programming-language-i
http://www.codejury.com/a-walk-in-x64-land/
http://www.d.umn.edu/~rmaclin/cs5641/Notes/L19_CodeGenerationI.pdf
http://www.incubatorgames.com/20110621/simple-scripting-language-part-5/
http://byteworm.com/2010/11/21/the-fastest-vm-bytecode-interpreter/
https://www.youtube.com/watch?v=OjaAToVkoTw
http://journal.stuffwithstuff.com/2013/12/08/babys-first-garbage-collector/
http://bartoszsypytkowski.com/simple-virtual-machine/
https://github.com/gphat/babyvm/blob/master/babyvm.c
https://en.wikibooks.org/wiki/X86_Disassembly/Functions_and_Stack_Frames
https://en.wikipedia.org/wiki/Lambda_lifting
https://en.wikipedia.org/wiki/Call_stack#Structure
https://en.wikipedia.org/wiki/Nested_function#Implementation

# Licence
Copyright (c) Alexander Koch 2015 All Rights Reserved.
Inception at 18.05.2015
Based on Project Ozone
