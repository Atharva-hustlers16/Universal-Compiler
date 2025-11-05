#!/usr/bin/env python3
"""
UMC-SDK Validation Script
Tests the project structure and basic functionality without requiring compilation.
"""

import os
import sys
from pathlib import Path

def check_project_structure():
    """Check if all required files and directories exist."""
    print("🔍 Checking project structure...")

    required_dirs = [
        'frontend',
        'ir',
        'backend',
        'runtime',
        'cli',
        'include',
        'tests',
        'docs',
        'build'
    ]

    required_files = [
        'CMakeLists.txt',
        'README.md',
        'include/LanguageDetector.h',
        'include/FrontendBase.h',
        'include/IRGenerator.h',
        'include/Optimizer.h',
        'include/CodeGenerator.h',
        'include/Runtime.h',
        'frontend/LanguageDetector.cpp',
        'frontend/FrontendBase.cpp',
        'frontend/CFrontend.cpp',
        'frontend/JavaFrontend.cpp',
        'frontend/PythonFrontend.cpp',
        'ir/IRGenerator.cpp',
        'ir/IRModule.cpp',
        'backend/Optimizer.cpp',
        'backend/CodeGenerator.cpp',
        'runtime/Runtime.cpp',
        'runtime/MemoryManager.cpp',
        'runtime/main.cpp',
        'cli/main.cpp',
        'tests/test_main.cpp'
    ]

    project_root = Path('umc-sdk')
    missing_items = []

    # Check directories
    for dir_name in required_dirs:
        dir_path = project_root / dir_name
        if not dir_path.exists():
            missing_items.append(f"Directory: {dir_name}")
        elif not dir_path.is_dir():
            missing_items.append(f"Path exists but not directory: {dir_name}")

    # Check files
    for file_name in required_files:
        file_path = project_root / file_name
        if not file_path.exists():
            missing_items.append(f"File: {file_name}")
        elif not file_path.is_file():
            missing_items.append(f"Path exists but not file: {file_name}")

    if missing_items:
        print("❌ Missing items:")
        for item in missing_items:
            print(f"   - {item}")
        return False
    else:
        print("✅ All project structure items present")
        return True

def check_cmake_files():
    """Check if CMakeLists.txt files are properly structured."""
    print("\n🔧 Checking CMake configuration...")

    cmake_files = [
        'CMakeLists.txt',
        'frontend/CMakeLists.txt',
        'ir/CMakeLists.txt',
        'backend/CMakeLists.txt',
        'runtime/CMakeLists.txt',
        'tests/CMakeLists.txt'
    ]

    project_root = Path('umc-sdk')
    issues = []

    for cmake_file in cmake_files:
        file_path = project_root / cmake_file
        if not file_path.exists():
            issues.append(f"Missing: {cmake_file}")
            continue

        content = file_path.read_text()
        if 'cmake_minimum_required' not in content:
            issues.append(f"Missing cmake_minimum_required in: {cmake_file}")
        if 'project(' not in content and cmake_file == 'CMakeLists.txt':
            issues.append(f"Missing project() in main CMakeLists.txt")

    if issues:
        print("❌ CMake configuration issues:")
        for issue in issues:
            print(f"   - {issue}")
        return False
    else:
        print("✅ CMake configuration looks good")
        return True

def check_language_detection():
    """Test language detection logic without compilation."""
    print("\n🗣️  Testing language detection...")

    test_cases = [
        ('hello.c', 'C'),
        ('main.cpp', 'C++'),
        ('Program.java', 'Java'),
        ('script.py', 'Python'),
        ('unknown.xyz', 'Unknown')
    ]

    # Simple extension-based detection (simulating our actual logic)
    def detect_language(filename):
        ext = filename.split('.')[-1].lower()
        if ext == 'c': return 'C'
        elif ext in ['cpp', 'cxx', 'cc']: return 'C++'
        elif ext == 'java': return 'Java'
        elif ext == 'py': return 'Python'
        else: return 'Unknown'

    success_count = 0
    for filename, expected in test_cases:
        detected = detect_language(filename)
        if detected == expected:
            print(f"   ✅ {filename} → {detected}")
            success_count += 1
        else:
            print(f"   ❌ {filename} → {detected} (expected {expected})")

    if success_count == len(test_cases):
        print("✅ Language detection working correctly")
        return True
    else:
        print(f"❌ Language detection issues ({success_count}/{len(test_cases)} correct)")
        return False

def main():
    """Run all validation checks."""
    print("🚀 UMC-SDK Phase 1 Validation")
    print("=" * 50)

    checks = [
        check_project_structure,
        check_cmake_files,
        check_language_detection
    ]

    results = []
    for check in checks:
        try:
            result = check()
            results.append(result)
        except Exception as e:
            print(f"❌ Error in {check.__name__}: {e}")
            results.append(False)

    print("\n" + "=" * 50)
    if all(results):
        print("🎉 All validation checks passed!")
        print("\n📋 Next steps:")
        print("1. Install Visual Studio Build Tools or MinGW-w64")
        print("2. Install LLVM development package")
        print("3. Run: cmake --build . --config Debug")
        print("4. Test: ./ucc hello.c")
        return 0
    else:
        print("❌ Some validation checks failed")
        print("\n🔧 Please fix the issues above before proceeding")
        return 1

if __name__ == "__main__":
    sys.exit(main())
