class Account
	def initialize(balance)
		@balance = balance
	end
	
	def withdraw(amount)
		@balance = @balance - amount
	end
end

account = Account.new(1000)
account.withdraw(100)