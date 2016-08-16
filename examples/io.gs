# Example of the file API.
# Copyright (c) Alexander Koch 2015
using core
using io

let file = File("vars.gs")
let content = file.read()
println(content)
