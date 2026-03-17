// Complex C test
#include <stdio.h>

int add(int a, int b) {
    return a + b;
}

int multiply(int a, int b) {
    return a * b;
}

int main() {
    int x = 5 + 3;
    int y = x * 2;
    int z = add(x, y);
    int w = multiply(z, 4);
    
    printf("Complex C test:\n");
    printf("x = %d\n", x);
    printf("y = %d\n", y);
    printf("z = %d\n", z);
    printf("w = %d\n", w);
    
    return 0;
}
