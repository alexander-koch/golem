# Example of a simple file API.
# Copyright (c) Alexander Koch 2015

using core
using io

# Example file implementation
# Future: gets merged into io lib
type File(name:char[]) {
	@Getter
	let filename = name

	func read() -> char[] {
		let content = readFile(filename)
		return content
	}

	func write(str:char[]) -> void {
		writeFile(filename, str, "wb")
	}

	func append(str:char[]) -> void {
		writeFile(filename, str, ["a"])
	}
}

let file = File("vars.gs")
let content = file.read()
println(content)
