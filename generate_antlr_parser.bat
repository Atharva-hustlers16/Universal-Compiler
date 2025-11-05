@echo off
REM ANTLR Integration Script for UMC-SDK (Windows)
REM Generates C++ parser from Java grammar file

echo Setting up ANTLR Integration for UMC-SDK...

REM Check if ANTLR is installed
where antlr4 >nul 2>&1
if %errorlevel% neq 0 (
    echo ❌ ANTLR4 not found. Please install ANTLR4:
    echo    Download from: https://www.antlr.org/download.html
    echo    Or install via package manager:
    echo    Chocolatey: choco install antlr4
    echo    Or download antlr-4.x-complete.jar manually
    echo.
    echo If using manual JAR, set ANTLR_JAR environment variable
    goto :error
)

REM Check if Java is available (ANTLR requirement)
where java >nul 2>&1
if %errorlevel% neq 0 (
    echo ❌ Java not found. ANTLR requires Java to run.
    goto :error
)

echo ✅ ANTLR4 and Java found

REM Generate ANTLR parser from grammar file
echo 📝 Generating ANTLR parser from Java.g4...

REM Create output directory for generated files
if not exist "generated\antlr" mkdir "generated\antlr"

REM Generate C++ parser
antlr4 -Dlanguage=Cpp -o generated/antlr -package umc Java.g4

if %errorlevel% equ 0 (
    echo ✅ ANTLR parser generated successfully in generated/antlr/
    echo.
    echo 📋 Generated files:
    dir /b "generated\antlr"
    echo.
    echo 🔧 Next steps:
    echo 1. Copy generated files to appropriate directories
    echo 2. Update CMakeLists.txt to include ANTLR runtime
    echo 3. Update JavaFrontend.cpp to use generated parser
    echo 4. Build and test the integration
) else (
    echo ❌ ANTLR parser generation failed
    goto :error
)

goto :eof

:error
echo.
echo ❌ ANTLR integration setup failed
exit /b 1
