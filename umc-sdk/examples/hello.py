# Example Python program demonstrating UMC-SDK capabilities

import os
import sys
from collections import defaultdict

def calculate_optimizations():
    """Test function with optimization opportunities"""
    # Constant folding opportunities
    x = 2 + 3          # Should be folded to 5
    y = x * 1          # Should be simplified to x  
    z = y - 0          # Should be simplified to y
    w = 4 * 0          # Should be folded to 0
    
    return x + y + z + w

def process_data(data):
    """Function that could be inlined"""
    result = []
    for item in data:
        if item is not None:
            # Algebraic simplification opportunity
            doubled = item * 2
            result.append(doubled)
    return result

def main():
    print("Hello from UMC-SDK Python Compiler!")
    
    # Test optimizations
    result = calculate_optimizations()
    print(f"Optimized calculation result: {result}")
    
    # Test data processing
    data = [1, 2, 3, 4, 5]
    processed = process_data(data)
    print(f"Processed data: {processed}")
    
    # Test control flow
    if len(sys.argv) > 1:
        print(f"Arguments: {len(sys.argv) - 1}")
    else:
        print("No arguments provided")
    
    # Test import handling
    print(f"Current directory: {os.getcwd()}")
    
    # Test class and method
    calculator = Calculator()
    calc_result = calculator.add(10, 20)
    print(f"Calculator result: {calc_result}")

class Calculator:
    """Test class for parsing"""
    
    def __init__(self):
        self.value = 0
    
    def add(self, a, b):
        """Method that could be optimized"""
        self.value = a + b
        return self.value
    
    def multiply(self, a, b):
        """Another method"""
        return a * b

if __name__ == "__main__":
    main()
