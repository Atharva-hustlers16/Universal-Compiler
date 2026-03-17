# Complex Python test
def add(a, b):
    return a + b

def multiply(a, b):
    return a * b

def main():
    x = 5 + 3
    y = x * 2
    z = add(x, y)
    w = multiply(z, 4)
    
    print("=== Complex Python test ===")
    print(f"x = {x}")
    print(f"y = {y}")
    print(f"z = {z}")
    print(f"w = {w}")
    return 0
