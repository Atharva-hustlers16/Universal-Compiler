package examples;

// Example Java program demonstrating UMC-SDK capabilities
public class HelloWorld {
    
    // Test method for parsing and optimization
    public static int calculate() {
        // These should be optimized by constant folding
        int a = 10 + 20;     // Should become 30
        int b = a * 1;        // Should become a
        int c = b - 0;        // Should become b
        int d = 5 * 0;        // Should become 0
        
        return a + b + c + d; // Should be optimized
    }
    
    // Test method that could be inlined
    private static int multiply(int x, int y) {
        return x * y;
    }
    
    public static void main(String[] args) {
        System.out.println("Hello from UMC-SDK Java Compiler!");
        
        int result = calculate();
        System.out.println("Calculated result: " + result);
        
        int product = multiply(result, 2);
        System.out.println("Product: " + product);
        
        // Test control flow
        if (args.length > 0) {
            System.out.println("Arguments provided: " + args.length);
        } else {
            System.out.println("No arguments provided");
        }
    }
}
