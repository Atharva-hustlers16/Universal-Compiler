#include <iostream>

class TestClass {
public:
    int value;
    TestClass(int v) : value(v) {}
};

int main() {
    TestClass obj(42);
    cout << "Simple class test" << endl;
    return 0;
}
