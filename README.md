# Universal Multi-Language Compiler SDK (UMC-SDK)

A system software similar to the JDK, but capable of compiling and executing code written in multiple programming languages (C, C++, Java, Python) within one integrated framework.

## Architecture Overview

UMC-SDK is organized into modular components that work together to provide a unified compilation and execution environment:

```
┌─────────────────────────────────────────────────────────────────┐
│                           UMC-SDK                               │
├─────────────────────────────────────────────────────────────────┤
│  CLI (ucc, urun)     │   Runtime     │   Frontend Parsers      │
│  ┌─────────────┐     │  ┌─────────┐  │  ┌──────┐ ┌──────┐       │
│  │ ucc (compile)│     │  │ uRuntime │  │  │  C   │ │ Java │       │
│  │ urun (run)  │◄────┤  │ Memory  │  │  │Clang │ │ANTLR │       │
│  └─────────────┘     │  │ I/O     │  │  │Parser│ │Parser│       │
│                      │  └─────────┘  │  └──────┘ └──────┘       │
├─────────────────────────────────────────────────────────────────┤
│  Backend Code Generation  │  IR Layer  │  Optimizer              │
│  ┌─────────────────────┐  │  ┌──────┐  │  ┌────────────────────┐  │
│  │ LLVM Target Code    │  │  │LLVM  │  │  │ LLVM Optimization  │  │
│  │ Generation          │◄─┤  │ IR   │◄─┤  │ Passes             │  │
│  └─────────────────────┘  │  └──────┘  │  └────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
```

## Module Structure

### Frontend Module (`/frontend`)
- **LanguageDetector**: Detects programming language from file extension and content
- **FrontendBase**: Abstract base class for language-specific parsers
- **CFrontend**: C/C++ parser using libclang
- **JavaFrontend**: Java parser using ANTLR grammar
- **PythonFrontend**: Python parser using Tree-sitter

### IR Layer (`/ir`)
- **IRGenerator**: Converts AST to LLVM Intermediate Representation
- **IRModule**: Manages LLVM modules and functions

### Backend (`/backend`)
- **Optimizer**: Applies LLVM optimization passes (constant folding, loop unrolling, DCE)
- **CodeGenerator**: Generates native machine code for target architecture

### Runtime (`/runtime`)
- **Runtime**: Execution environment for compiled binaries
- **MemoryManager**: Memory allocation and management for runtime

### CLI (`/cli`)
- **ucc**: Compiler tool (`ucc <file>` → compile source file)
- **urun**: Runtime executor (`urun <executable>` → execute compiled binary)

## Compilation Flow

1. **Language Detection**: File extension and content analysis
2. **Frontend Parsing**: Language-specific AST generation
3. **IR Generation**: AST → LLVM IR conversion
4. **Optimization**: LLVM optimization passes
5. **Code Generation**: IR → Native machine code
6. **Runtime Execution**: Load and execute compiled binary

## Build System

The project uses CMake for cross-platform builds:

```bash
mkdir build && cd build
cmake ..
make
```

## Development Status

### Phase 1 (Current): Structure & Functional Base ✅
- ✅ Project structure and CMake build system
- ✅ Skeleton classes for all modules
- ✅ Basic CLI driver with IR generation
- ✅ Language detection module
- ✅ LLVM integration

### Phase 2 (Next): Complete Functional System
- 🔄 Full frontend implementations (libclang, ANTLR, Tree-sitter)
- 🔄 Complete IR generation and optimization
- 🔄 Backend code generation and runtime execution
- 🔄 Comprehensive testing and validation

## Usage Examples

```bash
# Compile C code
./ucc hello.c

# Compile and run Python script
./ucc script.py

# Execute compiled binary
./urun hello.exe
```

## Dependencies

- **LLVM 17+**: For IR generation, optimization, and code generation
- **CMake 3.16+**: Build system
- **libclang**: C/C++ frontend (planned)
- **ANTLR**: Java frontend (planned)
- **Tree-sitter**: Python frontend (planned)

## Contributing

1. Follow the modular architecture
2. Add comprehensive tests for new features
3. Update documentation for API changes
4. Ensure cross-platform compatibility

## License

MIT License - See LICENSE file for details.
