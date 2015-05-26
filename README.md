# Project Nemesis

A functional programming language written in C.
The language for the pure evil.

(Still not clear if using Golem or Nemesis as Name for the language)

# Concept

Each variable is immutable at first. Variables are defined / declared using the 'let' keyword.
Example:

	let x = 5

Variables can be modified, if they are declared using the mod keyword.

	let mod x = 5
	x = x + 1

Functions are declared using the 'fn' keyword. Every function is then followed by a tuple.
The use of egyptian style braces is standard.

	fn main(args) {
		# body
	}

# Licence
Copyright (c) Alexander Koch 2015
Inception at 18.05.2015
