// Example C program demonstrating UMC-SDK capabilities
#include <stdio.h>

int main() {
    printf("Hello from UMC-SDK C Compiler!\n");
    
    // Test arithmetic operations that can be optimized
    int x = 2 + 3;        // Should be folded to 5
    int y = x * 1;         // Should be simplified to x
    int z = y - 0;         // Should be simplified to y
    int w = 4 * 0;         // Should be folded to 0
    
    printf("Optimized results: x=%d, y=%d, z=%d, w=%d\n", x, y, z, w);
    
    // Function that could be inlined
    int add(int a, int b) {
        return a + b;
    }
    
    int result = add(x, z);
    printf("add(%d, %d) = %d\n", x, z, result);
    
    return 0;
}
