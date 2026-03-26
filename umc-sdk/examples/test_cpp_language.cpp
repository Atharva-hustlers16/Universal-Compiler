#include <iostream>
using namespace std;

class TestClass {
public:
    int value;
    TestClass(int v) : value(v) {}
    int getValue() { return value; }
};

int main() {
    TestClass obj(42);
    cout << "C++ language test: " << obj.getValue() << endl;
    return 0;
}
