#!/bin/bash

# ANTLR Integration Script for UMC-SDK
# Generates C++ parser from Java grammar file

set -e

echo "🔧 Setting up ANTLR Integration for UMC-SDK..."

# Check if ANTLR is installed
if ! command -v antlr4 &> /dev/null; then
    echo "❌ ANTLR4 not found. Please install ANTLR4:"
    echo "   Download from: https://www.antlr.org/download.html"
    echo "   Or install via package manager:"
    echo "   Ubuntu/Debian: sudo apt install antlr4"
    echo "   macOS: brew install antlr"
    exit 1
fi

# Check if Java is available (ANTLR requirement)
if ! command -v java &> /dev/null; then
    echo "❌ Java not found. ANTLR requires Java to run."
    exit 1
fi

echo "✅ ANTLR4 and Java found"

# Generate ANTLR parser from grammar file
echo "📝 Generating ANTLR parser from Java.g4..."

# Create output directory for generated files
mkdir -p generated/antlr

# Generate C++ parser (change to Java target if needed)
antlr4 -Dlanguage=Cpp -o generated/antlr -package umc Java.g4

if [ $? -eq 0 ]; then
    echo "✅ ANTLR parser generated successfully in generated/antlr/"
    echo ""
    echo "📋 Generated files:"
    ls -la generated/antlr/
    echo ""
    echo "🔧 Next steps:"
    echo "1. Copy generated files to appropriate directories"
    echo "2. Update CMakeLists.txt to include ANTLR runtime"
    echo "3. Update JavaFrontend.cpp to use generated parser"
    echo "4. Build and test the integration"
else
    echo "❌ ANTLR parser generation failed"
    exit 1
fi
