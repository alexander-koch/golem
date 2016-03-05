public class fib {
	
	public static void main(String[] args) {
		new fib().start();
	}
	
	public int fib(int n) {
		if(n < 2) return n;
		return fib(n-2) + fib(n-1);
	}
	
	public void start() {
		long t0 = System.nanoTime();
		int i = 0;
		while(i < 5) {
			System.out.println(fib(28));
			i++;
		}
		long elapsed = System.nanoTime() - t0;
		System.out.println("Elapsed: "+elapsed/1e9);
	}
}