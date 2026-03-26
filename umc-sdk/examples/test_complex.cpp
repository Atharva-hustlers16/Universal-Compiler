#include <iostream>
using namespace std;

class Calculator {
private:
    int value;
    
public:
    Calculator(int val) : value(val) {}
    
    int add(int a, int b) {
        return a + b;
    }
    
    int multiply(int a, int b) {
        return a * b;
    }
    
    int getValue() {
        return value;
    }
};

int main() {
    Calculator calc(5);
    int x = 5 + 3;
    int y = x * 2;
    int z = calc.add(x, y);
    int w = calc.multiply(z, 4);
    
    cout << "=== Complex C++ Test ===" << endl;
    cout << "x = 5 + 3 = " << x << endl;
    cout << "y = x * 2 = " << y << endl;
    cout << "z = add(x, y) = " << z << endl;
    cout << "w = multiply(z, 4) = " << w << endl;
    cout << "Calculator value = " << calc.getValue() << endl;
    
    return 0;
}
