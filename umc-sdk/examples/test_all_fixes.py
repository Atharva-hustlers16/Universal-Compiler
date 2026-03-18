# Test all Python fixes: f-strings, str(), int(), float(), len()
def main():
    x = 10
    y = 3.14
    text = "Hello"
    
    # Test f-string (should be converted to concatenation)
    print(f"x = {x}")
    
    # Test str() function
    str_x = str(x)
    print("str(x) = " + str_x)
    
    # Test int() function
    int_y = int(y)
    print("int(3.14) = " + str(int_y))
    
    # Test float() function
    float_x = float(x)
    print("float(10) = " + str(float_x))
    
    # Test len() function
    len_text = len(text)
    print("len('Hello') = " + str(len_text))
    
    # Test nested function calls
    result = len(str(x))
    print("len(str(10)) = " + str(result))
    
    return 0
