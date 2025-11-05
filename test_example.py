#!/usr/bin/env python3

def calculate_sum(a, b):
    """Calculate the sum of two numbers."""
    result = a + b
    return result

class Calculator:
    def __init__(self, name):
        self.name = name
        self.history = []

    def add(self, x, y):
        result = x + y
        self.history.append(f"{x} + {y} = {result}")
        return result

    def get_history(self):
        return self.history

if __name__ == "__main__":
    # Test function
    sum_result = calculate_sum(5, 10)
    print(f"Function result: {sum_result}")

    # Test class
    calc = Calculator("MyCalc")
    class_result = calc.add(3, 7)
    print(f"Class result: {class_result}")

    # Print history
    print("History:")
    for item in calc.get_history():
        print(f"  {item}")
