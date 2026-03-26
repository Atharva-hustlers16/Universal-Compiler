# 🧪 Individual Language Testing Commands

## 📋 **Quick Reference - Test Each Language**

### **🔴 C Language Tests**
```bash
# Simple C Test
cd /d "D:\Univ Comp\umc-sdk"
full-build\Release\ucc.exe examples\test_simple.c --run

# Complex C Test  
full-build\Release\ucc.exe examples\test_complex.c --run

# C AST Only (No Execution)
full-build\Release\ucc.exe examples\test_complex.c
```

### **🐍 Python Language Tests**
```bash
# Simple Python Test
cd /d "D:\Univ Comp\umc-sdk"
full-build\Release\ucc.exe examples\test_simple.py --run

# Complex Python Test (with f-strings)
full-build\Release\ucc.exe examples\test_complex.py --run

# Python Built-in Functions Test
full-build\Release\ucc.exe examples\test_fixes_simple.py --run

# Python str() Function Test
full-build\Release\ucc.exe examples\test_str_function.py --run

# Python AST Only
full-build\Release\ucc.exe examples\test_complex.py
```

### **☕ Java Language Tests**
```bash
# Simple Java Test
cd /d "D:\Univ Comp\umc-sdk"
full-build\Release\ucc.exe examples\test_simple.java --run

# Complex Java Test (multiple methods)
full-build\Release\ucc.exe examples\test_complex.java --run

# Java AST Only
full-build\Release\ucc.exe examples\test_complex.java
```

---

## 🎯 **Complete Language Test Suite**

### **Option 1: Sequential Testing**
```bash
cd /d "D:\Univ Comp\umc-sdk"

echo "=== TESTING C LANGUAGE ===" && full-build\Release\ucc.exe examples\test_complex.c --run

echo "=== TESTING PYTHON LANGUAGE ===" && full-build\Release\ucc.exe examples\test_fixes_simple.py --run

echo "=== TESTING JAVA LANGUAGE ===" && full-build\Release\ucc.exe examples\test_complex.java --run
```

### **Option 2: Quick Language Detection Test**
```bash
cd /d "D:\Univ Comp\umc-sdk"

# Test language detection
echo "=== C LANGUAGE DETECTION ===" && full-build\Release\ucc.exe examples\test_simple.c
echo "=== PYTHON LANGUAGE DETECTION ===" && full-build\Release\ucc.exe examples\test_simple.py  
echo "=== JAVA LANGUAGE DETECTION ===" && full-build\Release\ucc.exe examples\test_simple.java
```

---

## 📊 **Expected Results**

### **🔴 C Language Expected Output**
```bash
=== Complex C Test ===
x = 8
y = 16
z = 24
w = 96
```

### **🐍 Python Language Expected Output**
```bash
x = 10           # F-string working ✅
10              # str(10) working ✅
3               # int(3.14) working ✅
10.000000      # float(10) working ✅
5               # len("Hello") working ✅
2               # len(str(10)) working ✅
```

### **☕ Java Language Expected Output**
```bash
=== Complex Java Test ===
x = 5 + 3 = 8
y = x * 2 = 16
z = add(x, y) = 24
w = multiply(z, 4) = 96
diff = subtract(w, x) = 88
Chained operations: multiply(add(10,5), subtract(20,8)) = 180
=== All Java operations completed successfully ===
```

---

## 🔍 **Diagnostic Commands**

### **Language Detection Verification**
```bash
# Check if language detection works correctly
cd /d "D:\Univ Comp\umc-sdk"

echo "=== C Detection ===" && full-build\Release\ucc.exe examples\test_simple.c | findstr "Detected language"
echo "=== Python Detection ===" && full-build\Release\ucc.exe examples\test_simple.py | findstr "Detected language"
echo "=== Java Detection ===" && full-build\Release\ucc.exe examples\test_simple.java | findstr "Detected language"
```

### **AST Structure Verification**
```bash
# Generate AST for each language (no execution)
cd /d "D:\Univ Comp\umc-sdk"

echo "=== C AST Structure ===" && full-build\Release\ucc.exe examples\test_simple.c | findstr "Program ()"
echo "=== Python AST Structure ===" && full-build\Release\ucc.exe examples\test_simple.py | findstr "Program ()"
echo "=== Java AST Structure ===" && full-build\Release\ucc.exe examples\test_simple.java | findstr "Program ()"
```

---

## 🚨 **Troubleshooting Commands**

### **Build Verification**
```bash
# Check if build is successful
cd /d "D:\Univ Comp\umc-sdk"
cmake --build full-build --config Release

# Verify executable exists
dir full-build\Release\ucc.exe
```

### **Individual Language Debugging**
```bash
# Test C with verbose output
cd /d "D:\Univ Comp\umc-sdk"
full-build\Release\ucc.exe examples\test_simple.c --run --verbose

# Test Python with verbose output
full-build\Release\ucc.exe examples\test_simple.py --run --verbose

# Test Java with verbose output
full-build\Release\ucc.exe examples\test_simple.java --run --verbose
```

---

## 🎓 **Viva Demonstration Commands**

### **Quick Demo Sequence**
```bash
cd /d "D:\Univ Comp\umc-sdk"

echo "🔴 C LANGUAGE DEMONSTRATION" && full-build\Release\ucc.exe examples\test_complex.c --run && echo ""

echo "🐍 PYTHON LANGUAGE DEMONSTRATION" && full-build\Release\ucc.exe examples\test_fixes_simple.py --run && echo ""

echo "☕ JAVA LANGUAGE DEMONSTRATION" && full-build\Release\ucc.exe examples\test_complex.java --run && echo ""

echo "🎉 ALL LANGUAGES TESTED SUCCESSFULLY!"
```

### **Feature-Specific Demos**
```bash
cd /d "D:\Univ Comp\umc-sdk"

# Demo 1: Basic Functionality
echo "=== BASIC FUNCTIONALITY DEMO ===" && full-build\Release\ucc.exe examples\test_simple.c --run

# Demo 2: Advanced Python Features
echo "=== ADVANCED PYTHON FEATURES DEMO ===" && full-build\Release\ucc.exe examples\test_fixes_simple.py --run

# Demo 3: Complex Java Methods
echo "=== COMPLEX JAVA METHODS DEMO ===" && full-build\Release\ucc.exe examples\test_complex.java --run
```

---

## 📈 **Performance Testing**

### **Speed Test Each Language**
```bash
cd /d "D:\Univ Comp\umc-sdk"

echo "=== C Performance Test ==="
powershell "Measure-Command { full-build\Release\ucc.exe examples\test_complex.c --run }"

echo "=== Python Performance Test ==="  
powershell "Measure-Command { full-build\Release\ucc.exe examples\test_fixes_simple.py --run }"

echo "=== Java Performance Test ==="
powershell "Measure-Command { full-build\Release\ucc.exe examples\test_complex.java --run }"
```

---

## 🎯 **One-Command Test Suite**

### **Complete Test (All Languages)**
```bash
cd /d "D:\Univ Comp\umc-sdk" && echo "🧪 UMC-SDK COMPLETE LANGUAGE TEST SUITE" && echo "" && echo "🔴 Testing C Language..." && full-build\Release\ucc.exe examples\test_complex.c --run && echo "" && echo "🐍 Testing Python Language..." && full-build\Release\ucc.exe examples\test_fixes_simple.py --run && echo "" && echo "☕ Testing Java Language..." && full-build\Release\ucc.exe examples\test_complex.java --run && echo "" && echo "🎉 ALL LANGUAGE TESTS COMPLETED SUCCESSFULLY!"
```

### **AST Generation Test (All Languages)**
```bash
cd /d "D:\Univ Comp\umc-sdk" && echo "🔍 UMC-SDK AST GENERATION TEST" && echo "" && echo "🔴 C AST:" && full-build\Release\ucc.exe examples\test_simple.c && echo "" && echo "🐍 Python AST:" && full-build\Release\ucc.exe examples\test_simple.py && echo "" && echo "☕ Java AST:" && full-build\Release\ucc.exe examples\test_simple.java
```

---

## ✅ **Success Indicators**

### **What to Look For**
- ✅ **Language Detection**: "Detected language: C/Python/Java"
- ✅ **AST Generation**: "Program ()" structure visible
- ✅ **Function Definitions**: Methods properly parsed
- ✅ **Execution**: "main() execution completed"
- ✅ **Clean Exit**: "AST execution completed successfully"

### **Common Issues & Solutions**
- ❌ **"Could not open file"** → Check file path and existence
- ❌ **"Build failed"** → Run `cmake --build full-build --config Release`
- ❌ **"Undefined variable"** → Parser issue - try simpler test
- ❌ **"Unknown function"** → Built-in function not implemented

---

## 🚀 **Ready to Test**

**Choose your testing approach:**

1. **Quick Test**: Run individual language commands
2. **Complete Suite**: Use one-command test suite  
3. **Viva Demo**: Use demonstration sequences
4. **Debug Mode**: Use verbose troubleshooting

**All commands are ready to execute!** 🎉

---

*Last Updated: March 26, 2026*
*Status: All Language Tests Working ✅*
