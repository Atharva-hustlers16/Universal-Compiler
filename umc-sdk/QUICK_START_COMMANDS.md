# 🚀 UMC-SDK - Quick Start Commands

## 📋 **Direct Commands to Run Project**

### **🔧 Build Commands**

#### **1. Build the Project**
```bash
# Navigate to project directory
cd /d "D:\Univ Comp\umc-sdk"

# Build with CMake (Recommended - Basic Build)
cmake --build full-build --config Release

# Alternative: Clean build
cmake --build full-build --config Release --clean
```

#### **2. Verify Build Success**
```bash
# Check if executable exists
dir "full-build\Release\ucc.exe"

# Should show: ucc.exe with timestamp ~500KB
```

---

### **🧪 Test Commands**

#### **1. Test C Language**
```bash
# Simple C Test
full-build\Release\ucc.exe examples\test_simple.c --run

# Complex C Test
full-build\Release\ucc.exe examples\test_complex.c --run
```

#### **2. Test Python Language**
```bash
# Simple Python Test (with str() function)
full-build\Release\ucc.exe examples\test_str_function.py --run

# Complex Python Test (with f-strings)
full-build\Release\ucc.exe examples\test_complex.py --run

# All Python Fixes Test
full-build\Release\ucc.exe examples\test_fixes_simple.py --run
```

#### **3. Test Java Language**
```bash
# Simple Java Test
full-build\Release\ucc.exe examples\test_simple.java --run

# Complex Java Test
full-build\Release\ucc.exe examples\test_complex.java --run
```

---

### **🎯 Quick Demo Commands**

#### **1. One-Command Full Test Suite**
```bash
cd /d "D:\Univ Comp\umc-sdk" && echo "=== TESTING C ===" && full-build\Release\ucc.exe examples\test_complex.c --run && echo "=== TESTING PYTHON ===" && full-build\Release\ucc.exe examples\test_fixes_simple.py --run && echo "=== TESTING JAVA ===" && full-build\Release\ucc.exe examples\test_complex.java --run
```

#### **2. Generate AST Only (No Execution)**
```bash
# Generate AST for Python
full-build\Release\ucc.exe examples\test_complex.py

# Generate AST for C
full-build\Release\ucc.exe examples\test_complex.c

# Generate AST for Java
full-build\Release\ucc.exe examples\test_complex.java
```

---

### **📊 Expected Results**

#### **C Language Output**
```
Complex C test:
x = 8
y = 16
z = 24
w = 96
```

#### **Python Language Output**
```
x = 10           # F-string working ✅
10              # str(10) working ✅
3               # int(3.14) working ✅
10.000000      # float(10) working ✅
5               # len("Hello") working ✅
2               # len(str(10)) working ✅
```

#### **Java Language Output**
```
=== Complex Java test ===
x = 8
y = 16
z = 24
w = 96
```

---

### **🔍 Debug Commands**

#### **1. Verbose Mode**
```bash
# Show detailed parsing information
full-build\Release\ucc.exe examples\test_complex.py --run --verbose
```

#### **2. Test Individual Components**
```bash
# Test language detection only
full-build\Release\ucc.exe examples\test_complex.c --detect-language

# Test AST generation only
full-build\Release\ucc.exe examples\test_complex.py --ast-only
```

---

### **⚡ Performance Test Commands**

#### **1. Speed Test**
```bash
# Measure execution time
powershell "Measure-Command { full-build\Release\ucc.exe examples\test_complex.c --run }"
```

#### **2. Memory Test**
```bash
# Monitor memory usage (Windows Task Manager)
# Run: taskmgr.exe and monitor while executing:
full-build\Release\ucc.exe examples\test_complex.py --run
```

---

### **🛠️ Troubleshooting Commands**

#### **1. Check Build Status**
```bash
# Verify all components built successfully
cd /d "D:\Univ Comp\umc-sdk" && cmake --build full-build --config Release && echo "Build Status: SUCCESS"
```

#### **2. Clean and Rebuild**
```bash
# Full clean rebuild
cd /d "D:\Univ Comp\umc-sdk"
rmdir /s /q full-build
mkdir full-build
cd full-build
cmake -G "Visual Studio 17 2022" -A x64 .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

#### **3. Verify Dependencies**
```bash
# Check MSVC compiler
cl

# Check CMake
cmake --version

# Check Visual Studio
devenv /?
```

---

### **📚 Documentation Commands**

#### **1. View Project Report**
```bash
# Open complete project documentation
start "D:\Univ Comp\umc-sdk\PROJECT_REPORT.md"

# Open functionality test report
start "D:\Univ Comp\umc-sdk\FUNCTIONALITY_TEST_REPORT.md"

# Open Python fixes summary
start "D:\Univ Comp\umc-sdk\PYTHON_FIXES_SUMMARY.md"
```

#### **2. View README**
```bash
# Open main README
start "D:\Univ Comp\umc-sdk\README.md"
```

---

### **🎯 Viva Demonstration Commands**

#### **1. Quick Demo Sequence**
```bash
# Step 1: Show C working
echo "=== C LANGUAGE DEMO ===" && full-build\Release\ucc.exe examples\test_complex.c --run

# Step 2: Show Python f-strings working
echo "=== PYTHON F-STRINGS DEMO ===" && full-build\Release\ucc.exe examples\test_complex.py --run

# Step 3: Show Python built-in functions
echo "=== PYTHON BUILT-IN FUNCTIONS DEMO ===" && full-build\Release\ucc.exe examples\test_fixes_simple.py --run

# Step 4: Show Java working
echo "=== JAVA LANGUAGE DEMO ===" && full-build\Release\ucc.exe examples\test_complex.java --run
```

#### **2. AST Structure Demonstration**
```bash
# Show AST generation for each language
echo "=== C AST ===" && full-build\Release\ucc.exe examples\test_simple.c
echo "=== PYTHON AST ===" && full-build\Release\ucc.exe examples\test_simple.py
echo "=== JAVA AST ===" && full-build\Release\ucc.exe examples\test_simple.java
```

---

### **🏆 Success Indicators**

#### **✅ Build Success**
```
frontend.vcxproj -> D:\Univ Comp\umc-sdk\full-build\Release\frontend.lib
runtime_lib.vcxproj -> D:\Univ Comp\umc-sdk\full-build\Release\runtime_lib.lib
ucc.vcxproj -> D:\Univ Comp\umc-sdk\full-build\Release\ucc.exe
```

#### **✅ Test Success**
- All languages execute without errors
- Output matches expected results
- AST generation shows proper structure
- No "Undefined variable" or "Unknown function" errors

#### **✅ Performance Success**
- Execution time < 100ms for simple programs
- Memory usage < 10MB
- No crashes or hangs

---

## 🎯 **Quick Start Summary**

### **One Command to Rule Them All:**
```bash
cd /d "D:\Univ Comp\umc-sdk" && cmake --build full-build --config Release && full-build\Release\ucc.exe examples\test_fixes_simple.py --run
```

### **Expected Final Output:**
```
=== TESTING ALL PYTHON FIXES - SIMPLE ===
x = 10
10
3
10.000000
5
2
=== main() execution completed ===
AST execution completed successfully
```

**🎉 PROJECT IS READY FOR DEMONSTRATION!**

---

*Last Updated: March 18, 2026*
*Status: Production Ready ✅*
