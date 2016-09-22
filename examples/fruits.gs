using core

# This is some comment

type String(string: str) {
	@Getter
	@Setter
 	let mut content = string
}

let mango = String("mango")
let kiwi = String("kiwi")
let apple = String("apple")

let fruits = [mango, kiwi, apple]

if fruits.empty() {
	println("There are no fruits")
} else {
	print(fruits.length())
	println(" fruits are there")
}

mango.setContent("Hello World!")
let ch = mango.getContent()
println(ch)
