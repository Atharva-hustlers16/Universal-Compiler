# 🔧 UMC-SDK Build Commands Guide

## 📋 **Complete Build Process Documentation**

### **🚨 Quick Build (Most Common)**
```bash
# Navigate to project directory
cd /d "D:\Univ Comp\umc-sdk"

# Build the project (if full-build exists)
cmake --build full-build --config Release

# Test build success
full-build\Release\ucc.exe examples\test_simple.c --run
```

---

## 🏗️ **Full Rebuild Process**

### **Step 1: Clean and Create Build Directory**
```bash
# Navigate to project directory
cd /d "D:\Univ Comp\umc-sdk"

# Remove existing build directory (if exists)
rmdir /s /q full-build

# Create new build directory
mkdir full-build
cd full-build
```

### **Step 2: Configure CMake**
```bash
# Simple CMake configuration (recommended)
cmake ..

# Alternative: With specific generator (if needed)
cmake -G "Visual Studio 17 2022" -A x64 ..
```

### **Step 3: Build Project**
```bash
# Build in Release mode
cmake --build . --config Release

# Or from parent directory
cmake --build full-build --config Release
```

### **Step 4: Verify Build**
```bash
# Check if executable exists
dir full-build\Release\ucc.exe

# Should show: ucc.exe with ~230KB size
```

### **Step 5: Test Build**
```bash
# Quick functionality test
full-build\Release\ucc.exe examples\test_simple.c --run

# Expected output: "Simple C test: x = 10"
```

---

## 🎯 **One-Command Complete Rebuild**

### **Full Clean Rebuild (Copy-Paste Ready)**
```bash
cd /d "D:\Univ Comp\umc-sdk" && rmdir /s /q full-build 2>nul && mkdir full-build && cd full-build && cmake .. && cd .. && cmake --build full-build --config Release && echo "✅ BUILD COMPLETED" && full-build\Release\ucc.exe examples\test_simple.c --run
```

---

## 🧪 **Build Verification Commands**

### **Quick Health Check**
```bash
cd /d "D:\Univ Comp\umc-sdk"

# Check build status
echo "=== BUILD STATUS ===" && dir full-build\Release\ucc.exe >nul 2>&1 && echo "✅ Build exists" || echo "❌ Build missing"

# Test all languages
echo "=== TESTING ALL LANGUAGES ===" && full-build\Release\ucc.exe examples\test_simple.c --run && full-build\Release\ucc.exe examples\test_fixes_simple.py --run && full-build\Release\ucc.exe examples\test_complex.java --run
```

### **Component Verification**
```bash
cd /d "D:\Univ Comp\umc-sdk"

# Check all build outputs
echo "=== BUILD COMPONENTS ===" && dir full-build\Release\*.lib && echo "=== EXECUTABLE ===" && dir full-build\Release\ucc.exe
```

---

## 🚨 **Troubleshooting Build Issues**

### **Common Build Problems & Solutions**

#### **Problem 1: "Could not create named generator"**
```bash
# Solution: Use simple CMake configuration
cd /d "D:\Univ Comp\umc-sdk\full-build"
cmake ..
```

#### **Problem 2: "Build failed" errors**
```bash
# Solution: Clean rebuild
cd /d "D:\Univ Comp\umc-sdk"
rmdir /s /q full-build
mkdir full-build
cd full-build
cmake ..
cmake --build . --config Release
```

#### **Problem 3: "ucc.exe not found"**
```bash
# Solution: Verify build completion
cd /d "D:\Univ Comp\umc-sdk"
dir full-build\Release\ucc.exe
# If missing, rebuild using full rebuild process
```

#### **Problem 4: CMake configuration errors**
```bash
# Solution: Clear CMake cache
cd /d "D:\Univ Comp\umc-sdk\full-build"
del CMakeCache.txt
cmake ..
```

---

## 🔄 **Maintenance Commands**

### **Regular Build Updates**
```bash
# After code changes, just rebuild
cd /d "D:\Univ Comp\umc-sdk"
cmake --build full-build --config Release
```

### **Periodic Clean Build**
```bash
# Weekly or after major changes
cd /d "D:\Univ Comp\umc-sdk"
rmdir /s /q full-build
mkdir full-build
cd full-build
cmake ..
cd ..
cmake --build full-build --config Release
```

---

## 📊 **Build Expected Results**

### **Successful Build Indicators**
```
✅ CMake configuration completes without errors
✅ MSBuild shows "frontend.vcxproj -> frontend.lib"
✅ MSBuild shows "runtime_lib.vcxproj -> runtime_lib.lib"  
✅ MSBuild shows "ucc.vcxproj -> ucc.exe"
✅ ucc.exe size: ~230KB (229,888 bytes)
✅ Test execution: "Simple C test: x = 10"
```

### **Build Output Files**
```
full-build/
└── Release/
    ├── frontend.lib      (~50KB)
    ├── runtime_lib.lib   (~30KB)
    └── ucc.exe          (~230KB)
```

---

## 🎓 **Viva Preparation Build Commands**

### **Pre-Demonstration Build Check**
```bash
cd /d "D:\Univ Comp\umc-sdk"

echo "🔧 VIVA PREP BUILD CHECK" && echo ""
echo "1. Build Status:" && dir full-build\Release\ucc.exe >nul 2>&1 && echo "   ✅ Executable ready" || echo "   ❌ Need to build"
echo ""
echo "2. Quick Tests:" && full-build\Release\ucc.exe examples\test_simple.c --run >nul 2>&1 && echo "   ✅ C working" || echo "   ❌ C failed"
echo ""
echo "3. Full Demo:" && full-build\Release\ucc.exe examples\test_complex.c --run && full-build\Release\ucc.exe examples\test_fixes_simple.py --run && full-build\Release\ucc.exe examples\test_complex.java --run
```

---

## 📱 **Quick Reference Commands**

### **Essential Commands (Memorize These)**
```bash
# 1. Navigate to project
cd /d "D:\Univ Comp\umc-sdk"

# 2. Quick build (if exists)
cmake --build full-build --config Release

# 3. Full rebuild (if broken)
rmdir /s /q full-build && mkdir full-build && cd full-build && cmake .. && cd .. && cmake --build full-build --config Release

# 4. Test build
full-build\Release\ucc.exe examples\test_simple.c --run
```

---

## 🛠️ **Development Workflow**

### **Daily Development**
```bash
cd /d "D:\Univ Comp\umc-sdk"
# Make code changes
cmake --build full-build --config Release
full-build\Release\ucc.exe examples\test_simple.c --run
```

### **Before Viva/Demo**
```bash
cd /d "D:\Univ Comp\umc-sdk"
# Clean rebuild for best performance
rmdir /s /q full-build && mkdir full-build && cd full-build && cmake .. && cd .. && cmake --build full-build --config Release
# Test all languages
full-build\Release\ucc.exe examples\test_complex.c --run
full-build\Release\ucc.exe examples\test_fixes_simple.py --run
full-build\Release\ucc.exe examples\test_complex.java --run
```

---

## 📋 **Build Requirements Checklist**

### **Before Building**
- [ ] Visual Studio 2022 installed
- [ ] CMake 3.16+ available
- [ ] In correct directory: `D:\Univ Comp\umc-sdk`
- [ ] No other processes using build files

### **After Building**
- [ ] `ucc.exe` exists in `full-build\Release\`
- [ ] Size approximately 230KB
- [ ] C test runs successfully
- [ ] Python test runs successfully
- [ ] Java test runs successfully

---

## 🎯 **Emergency Commands**

### **If Everything Goes Wrong**
```bash
# Emergency complete reset
cd /d "D:\Univ Comp\umc-sdk"
rmdir /s /q full-build 2>nul
git clean -fd 2>nul
git checkout . 2>nul
mkdir full-build
cd full-build
cmake ..
cd ..
cmake --build full-build --config Release
```

### **Quick Test Without Build**
```bash
# Check if current build works
cd /d "D:\Univ Comp\umc-sdk"
full-build\Release\ucc.exe examples\test_simple.c --run && echo "✅ Build working" || echo "❌ Need to rebuild"
```

---

## 📞 **Build Support**

### **Build Log Locations**
- **CMake Configure**: `full-build\CMakeFiles\CMakeOutput.log`
- **CMake Errors**: `full-build\CMakeFiles\CMakeError.log`
- **Build Output**: MSBuild console output

### **Common Build Times**
- **Quick rebuild**: ~30 seconds
- **Full clean build**: ~2-3 minutes
- **Configuration only**: ~10 seconds

---

## 🏆 **Success Confirmation**

### **Build Success Message**
```
✅ BUILD COMPLETED SUCCESSFULLY!

📁 Build Directory: D:\Univ Comp\umc-sdk\full-build\Release\
📄 Executable: ucc.exe (229,888 bytes)
🧪 Test Status: All languages working
🎯 Ready for: Development, Testing, Viva Demo

Next Steps:
1. Test individual languages
2. Run complete test suite
3. Prepare for viva demonstration
```

---

## 📚 **Related Documents**

- **QUICK_START_COMMANDS.md** - Runtime testing commands
- **INDIVIDUAL_LANGUAGE_TESTS.md** - Language-specific tests
- **PROJECT_REPORT.md** - Complete project documentation
- **FUNCTIONALITY_TEST_REPORT.md** - Detailed test results

---

*Last Updated: March 26, 2026*
*Build Version: UMC-SDK v1.0.0*
*Status: Production Ready ✅*

---

**🎉 SAVE THIS DOCUMENT for future build reference!**
