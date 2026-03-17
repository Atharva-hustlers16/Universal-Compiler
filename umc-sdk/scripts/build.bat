@echo off
REM UMC-SDK Build Script for Windows

setlocal enabledelayedexpansion

REM Default values
set BUILD_TYPE=Release
set BUILD_DIR=out\build
set INSTALL_PREFIX=C:\UMC-SDK
set ENABLE_LLVM=ON
set ENABLE_TREE_SITTER=ON
set ENABLE_ANTLR=ON
set VERBOSE=OFF
set RUN_TESTS=ON
set RUN_BENCHMARKS=OFF
set INSTALL_AFTER_BUILD=OFF
set CREATE_PACKAGE=OFF
set CLEAN=OFF
set INSTALL_DEPS=OFF

REM Colors (limited support in Windows cmd)
set INFO=[INFO]
set SUCCESS=[SUCCESS]
set WARNING=[WARNING]
set ERROR=[ERROR]

REM Function to print status
echo %INFO% UMC-SDK Build Script for Windows
echo %INFO% ================================

REM Parse command line arguments
:parse_args
if "%~1"=="" goto :main
if "%~1"=="-t" (
    set BUILD_TYPE=%~2
    shift
    shift
    goto :parse_args
)
if "%~1"=="--type" (
    set BUILD_TYPE=%~2
    shift
    shift
    goto :parse_args
)
if "%~1"=="-d" (
    set BUILD_DIR=%~2
    shift
    shift
    goto :parse_args
)
if "%~1"=="--dir" (
    set BUILD_DIR=%~2
    shift
    shift
    goto :parse_args
)
if "%~1"=="-p" (
    set INSTALL_PREFIX=%~2
    shift
    shift
    goto :parse_args
)
if "%~1"=="--prefix" (
    set INSTALL_PREFIX=%~2
    shift
    shift
    goto :parse_args
)
if "%~1"=="--llvm" (
    set ENABLE_LLVM=ON
    shift
    goto :parse_args
)
if "%~1"=="--no-llvm" (
    set ENABLE_LLVM=OFF
    shift
    goto :parse_args
)
if "%~1"=="--tree-sitter" (
    set ENABLE_TREE_SITTER=ON
    shift
    goto :parse_args
)
if "%~1"=="--no-tree-sitter" (
    set ENABLE_TREE_SITTER=OFF
    shift
    goto :parse_args
)
if "%~1"=="--antlr" (
    set ENABLE_ANTLR=ON
    shift
    goto :parse_args
)
if "%~1"=="--no-antlr" (
    set ENABLE_ANTLR=OFF
    shift
    goto :parse_args
)
if "%~1"=="-v" (
    set VERBOSE=ON
    shift
    goto :parse_args
)
if "%~1"=="--verbose" (
    set VERBOSE=ON
    shift
    goto :parse_args
)
if "%~1"=="--test" (
    set RUN_TESTS=ON
    shift
    goto :parse_args
)
if "%~1"=="--no-test" (
    set RUN_TESTS=OFF
    shift
    goto :parse_args
)
if "%~1"=="--benchmark" (
    set RUN_BENCHMARKS=ON
    shift
    goto :parse_args
)
if "%~1"=="--install" (
    set INSTALL_AFTER_BUILD=ON
    shift
    goto :parse_args
)
if "%~1"=="--package" (
    set CREATE_PACKAGE=ON
    shift
    goto :parse_args
)
if "%~1"=="--clean" (
    set CLEAN=ON
    shift
    goto :parse_args
)
if "%~1"=="--install-deps" (
    set INSTALL_DEPS=ON
    shift
    goto :parse_args
)
if "%~1"=="-h" (
    goto :show_help
)
if "%~1"=="--help" (
    goto :show_help
)
echo %ERROR% Unknown option: %~1
goto :show_help

:main
echo %INFO% Build configuration:
echo   Build Type: %BUILD_TYPE%
echo   Build Directory: %BUILD_DIR%
echo   Install Prefix: %INSTALL_PREFIX%
echo   LLVM Support: %ENABLE_LLVM%
echo   Tree-sitter Support: %ENABLE_TREE_SITTER%
echo   ANTLR Support: %ENABLE_ANTLR%
echo   Verbose: %VERBOSE%
echo.

REM Check for MSYS2
where pacman >nul 2>nul
if %errorlevel% neq 0 (
    echo %ERROR% MSYS2 not found. Please install MSYS2 first.
    echo %INFO% Download from: https://www.msys2.org/
    exit /b 1
)

REM Check for CMake
where cmake >nul 2>nul
if %errorlevel% neq 0 (
    echo %ERROR% CMake not found. Please install CMake.
    exit /b 1
)

REM Check for Visual Studio
where cl >nul 2>nul
if %errorlevel% neq 0 (
    echo %WARNING% Visual Studio compiler not found in PATH.
    echo %INFO% Please run this script from Visual Studio Developer Command Prompt.
    echo %INFO% Or run: "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
)

REM Install dependencies if requested
if "%INSTALL_DEPS%"=="ON" (
    echo %INFO% Installing dependencies via MSYS2...
    pacman -S --noconfirm ^
        mingw-w64-x86_64-cmake ^
        mingw-w64-x86_64-llvm ^
        mingw-w64-x86_64-libtree-sitter ^
        mingw-w64-x86_64-antlr4-runtime-cpp
    
    if %errorlevel% neq 0 (
        echo %ERROR% Failed to install dependencies
        exit /b 1
    )
    echo %SUCCESS% Dependencies installed
)

REM Clean if requested
if "%CLEAN%"=="ON" (
    echo %INFO% Cleaning build directory...
    if exist "%BUILD_DIR%" (
        rmdir /s /q "%BUILD_DIR%"
    )
    echo %SUCCESS% Build cleaned
)

REM Configure build
echo %INFO% Configuring build...
set CMAKE_ARGS=-A x64 ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    -DCMAKE_INSTALL_PREFIX="%INSTALL_PREFIX%" ^
    -DENABLE_LLVM=%ENABLE_LLVM% ^
    -DENABLE_TREE_SITTER=%ENABLE_TREE_SITTER% ^
    -DENABLE_ANTLR=%ENABLE_ANTLR% ^
    -DCMAKE_VERBOSE_MAKEFILE=%VERBOSE%

cmake -B "%BUILD_DIR%" %CMAKE_ARGS%
if %errorlevel% neq 0 (
    echo %ERROR% CMake configuration failed
    exit /b 1
)
echo %SUCCESS% Build configured

REM Build project
echo %INFO% Building project...
if "%VERBOSE%"=="ON" (
    cmake --build "%BUILD_DIR%" --config %BUILD_TYPE% --verbose
) else (
    cmake --build "%BUILD_DIR%" --config %BUILD_TYPE% --parallel
)
if %errorlevel% neq 0 (
    echo %ERROR% Build failed
    exit /b 1
)
echo %SUCCESS% Build completed

REM Run tests
if "%RUN_TESTS%"=="ON" (
    echo %INFO% Running tests...
    cd "%BUILD_DIR%"
    ctest --output-on-failure --config %BUILD_TYPE%
    if %errorlevel% neq 0 (
        echo %ERROR% Some tests failed
        cd ..
        exit /b 1
    )
    cd ..
    echo %SUCCESS% All tests passed
)

REM Run benchmarks
if "%RUN_BENCHMARKS%"=="ON" (
    echo %INFO% Running benchmarks...
    cd "%BUILD_DIR%"
    "%BUILD_TYPE%\ucc.exe" --benchmark
    if %errorlevel% neq 0 (
        echo %WARNING% Some benchmarks failed
    )
    cd ..
    echo %SUCCESS% Benchmarks completed
)

REM Install if requested
if "%INSTALL_AFTER_BUILD%"=="ON" (
    echo %INFO% Installing project...
    cmake --install "%BUILD_DIR%" --config %BUILD_TYPE%
    if %errorlevel% neq 0 (
        echo %ERROR% Installation failed
        exit /b 1
    )
    echo %SUCCESS% Installation completed
)

REM Create package if requested
if "%CREATE_PACKAGE%"=="ON" (
    echo %INFO% Creating package...
    cd "%BUILD_DIR%"
    cpack --config CPackConfig.cmake
    if %errorlevel% neq 0 (
        echo %ERROR% Package creation failed
        cd ..
        exit /b 1
    )
    cd ..
    echo %SUCCESS% Package created
)

echo.
echo %SUCCESS% Build script completed successfully!
echo %INFO% Executable location: %BUILD_DIR%\%BUILD_TYPE%\ucc.exe
echo.
echo %INFO% Usage examples:
echo   %BUILD_DIR%\%BUILD_TYPE%\ucc.exe examples\hello.c
echo   %BUILD_DIR%\%BUILD_TYPE%\ucc.exe examples\HelloWorld.java
echo   %BUILD_DIR%\%BUILD_TYPE%\ucc.exe examples\hello.py --run

goto :end

:show_help
echo UMC-SDK Build Script for Windows
echo.
echo Usage: build.bat [OPTIONS]
echo.
echo OPTIONS:
echo     -t, --type TYPE         Build type (Debug^|Release^|RelWithDebInfo) [default: Release]
echo     -d, --dir DIR           Build directory [default: out\build]
echo     -p, --prefix PREFIX     Install prefix [default: C:\UMC-SDK]
echo     --llvm                  Enable LLVM support [default: ON]
echo     --no-llvm               Disable LLVM support
echo     --tree-sitter           Enable Tree-sitter support [default: ON]
echo     --no-tree-sitter        Disable Tree-sitter support
echo     --antlr                 Enable ANTLR support [default: ON]
echo     --no-antlr              Disable ANTLR support
echo     -v, --verbose           Verbose build
echo     --test                  Run tests [default: ON]
echo     --no-test               Skip tests
echo     --benchmark             Run benchmarks [default: OFF]
echo     --install               Install after build
echo     --package               Create package
echo     --clean                 Clean build directory
echo     --install-deps          Install dependencies
echo     -h, --help              Show this help message
echo.
echo EXAMPLES:
echo     build.bat                           # Default build
echo     build.bat -t Debug -v               # Debug build with verbose output
echo     build.bat --no-llvm --install       # Build without LLVM and install
echo     build.bat --install-deps             # Install dependencies and build
echo.
echo PREREQUISITES:
echo     1. MSYS2 (https://www.msys2.org/)
echo     2. Visual Studio 2019+ with C++ tools
echo     3. Run from Visual Studio Developer Command Prompt
echo.

:end
endlocal
