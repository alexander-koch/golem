# Neuron / Perceptron in Golem
# Copyright (c) Alexander Koch 2015
#
# Funktion:
# Das Programm fragt nach zwei Eingaben für den AND-Operator.
# Eingabemöglichkeiten sind jeweils -1.0 oder 1.0.
# Die entsprechende Antwort wird ausgegeben.
using core
using math

# Zufallsnummer in Bereich
func rand(a: float, b: float) -> float {
	return (b-a)*prng() + a
}

# Einzelnes Neuron definieren
type Neuron() {
	let mut weights = [::float]
	let c = 0.1

	# Erstellen
	func create(n: int) {
		let mut i = 0
		while i < n {
			weights := weights.add(rand(-1.0, 1.0))
			i := i + 1
		}
	}

	# Eingaben verarbeiten
	func feedForward(inputs: float[]) -> float {
		let mut sum = 0.0
		let mut i = 0
		while i < inputs.length() {
			sum := sum + inputs[i] * weights[i]
			i := i + 1
		}
		return tanh(sum)
	}

	# Neuron trainieren
	func train(inputs: float[], desired: float) {
		let guess = feedForward(inputs)
		let error = desired - guess
		let mut i = 0
		while i < weights.length() {
			weights[i] := weights[i] + c * error * inputs[i]
			i := i + 1
		}
	}
}

let neuron = Neuron()
neuron.create(3)

# AND-operator
let cond1 = [1.0, 1.0, 1.0]
let cond2 = [1.0, -1.0, 1.0]
let cond3 = [-1.0, 1.0, 1.0]
let cond4 = [-1.0, -1.0, 1.0]

let mut n = 0
while n < 10 {
	neuron.train(cond1, 1.0)
	neuron.train(cond2, -1.0)
	neuron.train(cond3, -1.0)
	neuron.train(cond4, -1.0)
	n := n + 1
}

println("Enter two values:")
print("V1: ")
let v1 = parseFloat(getline())
print("V2: ")
let v2 = parseFloat(getline())
let guess = neuron.feedForward([v1, v2, 1.0])
let result = (guess > 0.0)
println("Result: $result")
