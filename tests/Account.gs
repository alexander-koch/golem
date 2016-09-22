using core

type Account(_balance: int) {
	@Getter
	let mut balance = _balance

	func withdraw(amount: int) {
		balance := balance - amount
	}
}

# Create and use account
let account = Account(1000)
account.withdraw(100)
println(account.getBalance())
