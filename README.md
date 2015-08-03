# Project Golem-lang

A programming language written in C.

# Project aims

- Functional, object-oriented scripting / programming language
=> Immutability of objects
=> Everything is an object

- Statically typed, exception: variables, type is set by context
- Strong typing
- High-order functions, currying
- Fully functional fast bytecode vm or translation into x86 / x86_64 assembler machine code
- Systems language
- Python's "Batteries included" philosophy for standard libraries
- Garbage collected
- FAST!!!


# TODO

- Implementing all (basic) operators for associated types in the vm / bytecodes => (Done.)
- Finalize implementing functions => (Done.)
- mark-sweep garbage collector for vm => (Done.)
- Subscripts (Arrays / Strings) => (Working on it)
- Lambdas => (Prototype / Concept)

Langugage specific
===

- AST optimization
- prefix operators
- Lambdas
- Typecasting

Implementation specific
===

- map / filter / curry methods
- performance and bytecode optimization
- JVM approach:
	- compiler: source code => bytecodes => file
 	- vm: file => vm => execution

# Concept (final concept may change)

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

Functions are declared using the 'func' keyword.
The body is bound by egyptian style braces. You have to declare the parameters and the return type.
```rust
	func main(args: str[]) -> void {
		# body
	}
```

Classes are defined using the structure below. The methods `new` is the classes constructor.
All classes are objects, similar to Scala. All attributes are private.
To get field, getters are used to maintain encapsulation.
```rust
 	object Main {

		# Attributes
		let mut x = 2
		let mut y = 4

		# Constructor
		func new(x: int, y: int, z: int) -> void {
			this.x = 2
			this.y = 4
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
```
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
```rust
	while(number == 5) {
		number = number + 1
	}
```

Lambdas (Concept idea, not implemented, final result may change):
Lambda keyword followed by a parameter list, arrow and expression.
Lambdas should fit in one line, if bigger needed, use a function instead.
```rust
	func main(argc: int) -> void {
		return lambda(x) -> x + 2
	}
```

# Curently supported (AST)

- variable declaration (immutable/mutable)
- function declaration
- if statements
- while loop
- function calls
- array declaration
- subscripts
- expressions with precedence
- classes

# Currently supported (Langugage-based)

- variable and function declaration
- types int, string, float, bool, (void)
- function calls, recursion
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

# Licence
Copyright (c) Alexander Koch 2015
Inception at 18.05.2015
Based on Project Ozone
