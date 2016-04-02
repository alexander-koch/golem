
public class Account {
	private int balance;
	
	public Account(int balance) {
		this.balance = balance;
	}
	
	public void withdraw(int amount) {
		this.balance -= amount;
	}
	
	public static void main(String[] args) {
		new Account(1000).withdraw(100);
	}
}