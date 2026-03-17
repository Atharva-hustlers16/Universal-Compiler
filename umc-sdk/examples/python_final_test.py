# Enhanced Python test for UMC-SDK
def add(a, b):
    return a + b

def multiply(x, y):
    return x * y

def main():
    x = 10 + 5
    y = x * 2
    z = add(x, y)
    product = multiply(y, 2)
    
    print("=== UMC-SDK Python Demo ===")
    print(f"Arithmetic: x={x}, y={y}, z={z}")
    print(f"Functions: add({x}, {y})={z}, multiply({y}, 2)={product}")
    
    return 0
