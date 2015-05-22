# Project Nemesis

A pure functional programming language written in C.
The language for the pure evil.

(Still not clear if using Golem or Nemesis as Name for the language)

# Concept

Each variable is immutable at first. Variables are defined / declared using the 'let' keyword.
Example:

	let x = 5

Functions are also declared using the let keyword, but followed by a set.
A set can be a list of variables or a static value

 	let add {x,y} ::= x * y
	let add {1} ::= 2

# License
Copyright (c) Alexander Koch 2015
Inception at 18.05.2015