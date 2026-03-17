// Complex Java test
public class Calculator {
    public static int add(int a, int b) {
        return a + b;
    }
    
    public static int multiply(int a, int b) {
        return a * b;
    }
    
    public static void main(String[] args) {
        int x = 5 + 3;
        int y = x * 2;
        int z = add(x, y);
        int w = multiply(z, 4);
        
        System.out.println("=== Complex Java test ===");
        System.out.println("x = " + x);
        System.out.println("y = " + y);
        System.out.println("z = " + z);
        System.out.println("w = " + w);
    }
}
