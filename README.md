# Project Nemesis / Golem-lang

A programming language written in C.
The language for the pure evil.

# Concept

Each variable is immutable at first. Variables are defined / declared using the 'let' keyword.
Example:

	let x = 5

Variables are mutable, if they are declared using the 'mut' keyword.

	let mut x = 5
	x = x + 1

Functions are declared using the 'fn' keyword. Every function is then followed by a tuple.
The use of egyptian style braces is standard (Not supported yet, final structure may change).

	fn main(args) {
		# body
	}

Classes are defined using the following structure (Not supported yet, final structure may change):

 	class Main {

		let mut x = 2
		let mut y = 4

		# Constructor
		fn new(x, y, z) {
			self.x = 2
			self.y = y
		}

		fn run() {
			println("Running with x:", self.x, "and y:", self.y)
		}

		fn free() {
			# do some stuff here
		}
	}

# Curently supported (AST)

- variable declaration (immutable/mutable)
- function declaration
- if statements
- while loop
- function calls
- array declaration
- subscripts
- expressions with precedence

# TODO

- classes
- imports
- for loops
- unary- / prefix-operators fix (including pointers '*val = 5')
- implement bytecode vm / llvm compiler

# Pointer and ownership (TODO)
Ownership orients on Rusts system. Once you call a function with an object, ownership is transferred to that function.
The function deletes it at the end of the scope. To still own it after, you have to return it.
This prevents use of a pointer, after it has been freed.


# Licence
Copyright (c) Alexander Koch 2015
Inception at 18.05.2015
Based on Project Ozone
