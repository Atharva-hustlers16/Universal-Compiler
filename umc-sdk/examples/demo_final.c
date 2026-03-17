// Final demonstration of enhanced UMC-SDK AST execution
#include <stdio.h>

int add(int a, int b) {
    return a + b;
}

int multiply(int x, int y) {
    return x * y;
}

int main() {
    printf("=== UMC-SDK Enhanced AST Execution Demo ===\n");
    
    // Test basic arithmetic
    int x = 10 + 5;
    int y = x * 2;
    int z = y - 3;
    
    printf("Arithmetic: x=%d, y=%d, z=%d\n", x, y, z);
    
    // Test function calls
    int sum = add(x, z);
    int product = multiply(y, 2);
    
    printf("Functions: add(%d, %d)=%d, multiply(%d, %d)=%d\n", x, z, sum, y, 2, product);
    
    // Test string literals with escape sequences
    printf("String with tabs:\tand\nnewlines\n");
    
    return 0;
}
