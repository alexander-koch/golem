# Golem

Golem is a statically typed, object-oriented programming language.

# Example

```rust
using core

func fib(n: int) -> int {
	if n < 2 {
		return n
	}
	return fib(n-2) + fib(n-1)
}

let t0 = clock()
let mut i = 0
while i < 5 {
	println(fib(28))
	i := i + 1
}
let elapsed = clock() - t0
println("Elapsed: $elapsed")

```

# Features

- Easily readable and learnable
- **Normal** object-oriented features (classes, method calls, etc.)
- Immutablitiy of everything by default
- Explicit mutable declaration
- Statically, strong-typed
- Automatic type inference
- **NULL** is not possible to create or use
- Option types
- Fast, garbage collected bytecode VM

# Installing

Golem is written in C99, and has been tested to run on Linux and Windows.
A Makefile is used to build the compiler.

```sh
$ make
$ golem your/file/here.gs
```

Use `make DEBUG=1` to generate the debugger.
To generate the immediate representation tool, run `make ir`.
More features can be enabled/disabled in the Makefile.

# Info

For more information about the syntax see [Syntax](Syntax.md).
This project was used in the german science competition "Jugend forscht" 2016.

# Licence

Licensed under GNU GPLv3.
Copyright (c) Alexander Koch 2017
