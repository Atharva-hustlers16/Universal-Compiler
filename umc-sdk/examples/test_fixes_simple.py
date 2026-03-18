# Simple test for all Python fixes
def main():
    x = 10
    y = 3.14
    text = "Hello"
    
    # Test f-string (should be converted to concatenation)
    print(f"x = {x}")
    
    # Test str() function
    str_x = str(x)
    print(str_x)
    
    # Test int() function
    int_y = int(y)
    print(int_y)
    
    # Test float() function
    float_x = float(x)
    print(float_x)
    
    # Test len() function
    len_text = len(text)
    print(len_text)
    
    # Test nested function calls
    result = len(str(x))
    print(result)
    
    return 0
