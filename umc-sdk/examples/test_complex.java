// Complex Java test - Multi-class demonstration
public class Calculator {
    // Static method for addition
    public static int add(int a, int b) {
        return a + b;
    }
    
    // Static method for multiplication
    public static int multiply(int a, int b) {
        return a * b;
    }
    
    // Static method for subtraction
    public static int subtract(int a, int b) {
        return a - b;
    }
    
    // Main method to demonstrate all functionality
    public static void main(String[] args) {
        // Initialize variables with complex expressions
        int x = 5 + 3;
        int y = x * 2;
        int z = add(x, y);
        int w = multiply(z, 4);
        int diff = subtract(w, x);
        
        // Print results using System.out.println
        System.out.println("=== Complex Java Test ===");
        System.out.println("x = 5 + 3 = " + x);
        System.out.println("y = x * 2 = " + y);
        System.out.println("z = add(x, y) = " + z);
        System.out.println("w = multiply(z, 4) = " + w);
        System.out.println("diff = subtract(w, x) = " + diff);
        
        // Demonstrate method chaining
        int result = multiply(add(10, 5), subtract(20, 8));
        System.out.println("Chained operations: multiply(add(10,5), subtract(20,8)) = " + result);
        
        // Final status
        System.out.println("=== All Java operations completed successfully ===");
    }
}
