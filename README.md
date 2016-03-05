# Golem

Golem is a statically typed, object-oriented programming language.

# Features

- Easily readable and learnable, because of familiar syntax
- **Normal** object-oriented features (classes, method calls, etc.)
- Immutablitiy of everything by default
- Explicit mutable declaration
- Statically, strong-typed
- Automatic type inference
- Strings as arrays of characters, no extra type
- **NULL** is not possible to create or use
- Fast, garbage collected bytecode vm

# Installing

Golem is written in C99, and has been tested to run on Linux and Windows.
To build it, a Makefile is used.

	$ make release
	$ golem your/file/here.gs

If you are a developer and you want to improve Golem,
you can use the debug version, which prints out the abstract syntax tree, the bytecode and additional information.

	$ make debug
	§ golem-debug your/file/here.gs

More features for the debug-version can be enabled/disabled in the Makefile.

# TODO

#### Main tasks

##### Serious issues

- [ ] (Intermediate) While and if loops don't provide scopes => (Working on it.)

##### Tasks

- [ ] (Advanced) Standard libraries and external classes, DLL loading => (Working on it.)
- [ ] (Advanced) Bytecode optimizations (class optimizer -> remove unused code and functions!) => (Working on it.)
- [ ] (Advanced) Replace instruction_t vector by array of type val_t for faster processing => (Task.)
- [ ] (Intermediate) Do for loops => (Task)
- [ ] (Intermediate) Do namespaces => (Task)
- [ ] (Intermediate) Implement switch statements => (Task)

##### Done

- [x] (Easy) Add a few benchmarks => (Done.)
- [x] (Intermediate) Implement computed gotos to gain 15%-20% more speed => (Done.)
- [x] (Intermediate) Standard operators for internal classes => (Done.)
- [x] (Intermediate) Serializer for bytecodes => (Done.)
- [x] (Intermediate) Final fixes for arrays (NaN-Tagging) => (Done.)
- [x] (Advanced) Use NaN-Tagging and increase vm performance => (Done.)
- [x] (Intermediate) Create a annotation system => (Done.)
- [x] Move import to compiler side for individual errors => (Done.)
- [x] Import other files for bigger projects => (Done.)
- [x] Class functions allow access to constructor parameters => (Done.)
- [x] Array operators / Allow zero element arrays => (Done.)
- [x] Class field modification within a class function doesn't work as expected => (Done.)
- [x] Comment at end of file is invalid => (Done.)
- [x] Implement new string merge operator => (Done.)
- [x] New operator for reassignment (:=), replace double equal by equal => (Done.)
- [x] Return type isn't checked => (Done.)
- [x] Calling an interal function in a class yields an error => (Done.)
- [x] Classes / Objects parsing + compilation => (Done.)
- [x] Functions with Classes as parameters => (Done.)
- [x] When trying to replace an **Upvalue**, wrong storage address is used => (Done.)
- [x] Nested functions => (Done.)
- [x] Implementing all (basic) operators for associated types in the vm / bytecodes => (Done.)
- [x] Finalize implementing functions => (Done.)
- [x] mark-sweep garbage collector for vm => (Done.)
- [x] Mutable parameters => (Done.)
- [x] Subscripts (Arrays / Strings) => (Done.)

# Syntax

### Variables

A variable is declared using the `let` keyword and is set to immutable by default.
Because of the type-inference, there is no type declaration needed.
Example:
```rust
	let x = 5
```

Variables can be modified, if they are declared using the `mut` (mutable) keyword.
When modifiying a variable, the re-assignment operator `:=` is used.
```rust
	let mut x = 5
x := x + 1
```

Although you can change the value of the variable, you can **NOT** change its type.
The following types can be used:
```c
	int: 2, 4, 512, 128, 65535, ...
float: 3.14, 6.28, 0.2, 0.25, ...
char: "a", "b", "c", "d", "e", "f", ...
bool: "true", "false"
```

With classes you can also use custom types.
Arrays are also permitted.

### Arrays

Arrays are defined using two brackets, in which the content is placed and seperated by commas.
An array can only be of one datatype and cannot be replaced or modified if it is immutable.
For character arrays, a string initializer can be used (type `char[]`).

```rust
	let arr = [1,2,3,4,5]
let str = "Hello World" // Same as ["H", "E", "L", ...]
```

One wide strings / characters

```rust
	# Declares a character
let c1 = "r"

# Declares a string
let c2 = ["r"]
```

If you want to use empty arrays, you have to declare the type of them
using the following form: (EBNF)
```rust
	array = '[', '::', type, ']'
```

Example in code:
```rust
	let mut array = [::int]
```

This would create an empty, mutable integer-array.

### Functions

Functions are declared using the `func` keyword.
Then you have to declare the parameters with their type and finally a return type using the 'arrow' notation.
For code blocks the 'egyptian'-style brackets are used.
For void functions, you don't have to use the arrow, it is optional.

```rust
	func main(arg0:char[]) -> int {
	# body
}
```

Parameters are always immutable, if the `mut` keyword is not set.

```go
	func main(mut arg0:char[], arg1:int, arg2:float) -> float {
	arg0 := "Foobar"
	let answer = arg1
	return arg2
}

main("Foo", 42, 3.14)
```

### Classes

Classes are defined using the structure below.
All attributes are private.
To index or set a field, getters and setters are used, to maintain encapsulation.
The class declaration is also the class's constructor to prevent NULL-values during initialization.

```go
	using core

type Class(_x:int, _y:int, _z:int) {
	@Getter
	@Setter
	let mut x = _x
	let y = _y
	let z = _z

	func run() {
		println(x)
		println(y)
	}
}

func main() {
	let cls = Class(7,5,2)
	cls.run()
	println(cls.getX())
}

main()
```

### Control Flow

Control flows are created by if statements or while loops.
Example: (assuming variable 'number' is declared as an integer):
```rust
	if number = 5 {
	println("Your number is odd.")
} else if number = 3 {
	println("Your number is odd and it's three.")
} else {
	println("Your number isn't 5 or 3. You should feel bad.")
}
```
For equality the `=`-operator is used (not the double-equal `==` as in other programming languages).

While loops:
```rust
	while number = 5)  {
	number := number + 1
}
```

For future implementations, for loops should also be an option, but they are currently not implemented.
For loops (Ideas):
```ruby
	for |iter| in [1...5] {
	print(iter)
}

for |i| = 0; 1 < 5; i := i + 1 {
	print(i)
}

for(condition) {
	print("Condition is still true")
}

```

### Internal class functions

Everything is a class. Integers, characters and arrays have their own functions.

#### Integer:
```
to_f() -> float
to_c() -> char
to_str() -> char[]
```

#### Character:
```
to_f() -> float
to_i() -> int
to_str() -> char[]
```

#### Float:
```
to_i() -> int
to_c() -> float
to_str() -> char[]
```

#### Boolean:
```
to_i() -> int
to_str() -> char[]
```

#### Array:
```
length() -> int
emtpy() -> bool
append(other:T[]) -> T[]
cons(other:T) -> T[]
at(index:int) -> T
```

### Annotations

Until now, there are three different types of annotations: @Getter, @Setter and @Unused.
Getter and Setter can only be used within classes and are used to create getter- or setter-methods for private variables.
For example we create an attribute named myVar with the value x.

	let myVar = x

If we now want to access myVar outside of the class, the getter annotation is used like this:

	@Getter
	let myVar = x

Therefore the variable is accessed like this:

	let myClass = MyClass(5)
	let res = myClass.getMyVar()

Getter will automatically generate a method returning the value with the name of the variable as prefix.
In the case above get-MyVar-() is used. The same rule applies for the @Setter-annotation.
The first character of the variable name is converted to upper-case.

# Syntactic sugar

Subscripts can either be represented by brackets after an expression or by a dot.
Both are internally the same, so class fields can also be accessed by brackets, arrays also by a dot.

Example:

```ruby
	# MyClass x was declared with function getX()
let class = MyClass()

# Normal function call
class.getX()

#Syntactic sugar
class[getX]()
```

# Licence
Copyright (c) Alexander Koch 2016 All Rights Reserved.
Project start: 18.05.2015
