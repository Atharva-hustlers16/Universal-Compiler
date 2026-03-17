#include "Runtime.h"
#include "ASTInterpreter.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <map>
#include <algorithm>

Runtime::Runtime() : loadedModule_(nullptr), memoryManager_(nullptr), astInterpreter_(nullptr) {
    initializeRuntime();
}

Runtime::~Runtime() {
    cleanup();
    shutdownRuntime();
}

bool Runtime::initializeRuntime() {
    std::cout << "Initializing uRuntime environment..." << std::endl;

    // Initialize memory management
    memoryManager_ = new MemoryManager();
    if (!memoryManager_) {
        std::cerr << "Error: Failed to initialize memory manager" << std::endl;
        return false;
    }

    // Initialize I/O system
    ioManager_ = new IOManager();
    if (!ioManager_) {
        std::cerr << "Error: Failed to initialize I/O manager" << std::endl;
        delete memoryManager_;
        return false;
    }

    // Initialize execution context
    executionContext_ = new ExecutionContext();
    if (!executionContext_) {
        std::cerr << "Error: Failed to initialize execution context" << std::endl;
        delete memoryManager_;
        delete ioManager_;
        return false;
    }

    // Initialize AST interpreter
    astInterpreter_ = new ASTInterpreter();
    if (!astInterpreter_) {
        std::cerr << "Error: Failed to initialize AST interpreter" << std::endl;
        delete memoryManager_;
        delete ioManager_;
        delete executionContext_;
        return false;
    }

    std::cout << "uRuntime initialized successfully" << std::endl;
    return true;
}

void Runtime::shutdownRuntime() {
    std::cout << "Shutting down uRuntime environment..." << std::endl;

    if (executionContext_) {
        delete executionContext_;
        executionContext_ = nullptr;
    }

    if (ioManager_) {
        delete ioManager_;
        ioManager_ = nullptr;
    }

    if (memoryManager_) {
        delete memoryManager_;
        memoryManager_ = nullptr;
    }

    if (astInterpreter_) {
        delete astInterpreter_;
        astInterpreter_ = nullptr;
    }
}

bool Runtime::loadExecutable(const std::string& executablePath) {
    executablePath_ = executablePath;

    std::ifstream file(executablePath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open executable file: " << executablePath << std::endl;
        return false;
    }

    // Read executable into memory
    std::vector<char> buffer((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());

    // Parse executable format (simplified - in real implementation, parse ELF/PE)
    if (!parseExecutableFormat(buffer)) {
        std::cerr << "Error: Failed to parse executable format" << std::endl;
        return false;
    }

    std::cout << "Loaded executable: " << executablePath << std::endl;
    std::cout << "Executable size: " << buffer.size() << " bytes" << std::endl;

    return true;
}

bool Runtime::execute(int argc, char** argv) {
    if (executablePath_.empty()) {
        std::cerr << "Error: No executable loaded" << std::endl;
        return false;
    }

    std::cout << "Executing: " << executablePath_ << std::endl;

    // Set up execution environment
    if (!setupExecutionEnvironment(argc, argv)) {
        std::cerr << "Error: Failed to setup execution environment" << std::endl;
        return false;
    }

    // Execute the program
    int exitCode = executeProgram();

    std::cout << "Program execution completed with exit code: " << exitCode << std::endl;
    return exitCode == 0;
}

bool Runtime::executeAST(const ASTNode& ast) {
    if (!astInterpreter_) {
        std::cerr << "Error: AST interpreter not initialized" << std::endl;
        return false;
    }

    std::cout << "Executing AST directly..." << std::endl;
    std::cout << "AST to execute:" << std::endl;
    std::cout << ast.toString() << std::endl;

    bool success = astInterpreter_->execute(ast);
    
    if (success) {
        std::cout << "AST execution completed successfully" << std::endl;
    } else {
        std::cout << "AST execution failed" << std::endl;
    }
    
    return success;
}

void Runtime::cleanup() {
    if (loadedModule_) {
        // Cleanup loaded module resources
        loadedModule_ = nullptr;
    }

    if (executionContext_) {
        executionContext_->cleanup();
    }
}

void* Runtime::allocateMemory(size_t size) {
    if (!memoryManager_) {
        std::cerr << "Error: Memory manager not initialized" << std::endl;
        return nullptr;
    }

    void* ptr = memoryManager_->allocate(size);
    if (ptr) {
        std::cout << "Allocated " << size << " bytes at " << ptr << std::endl;
    }
    return ptr;
}

void Runtime::freeMemory(void* ptr) {
    if (!memoryManager_) {
        std::cerr << "Error: Memory manager not initialized" << std::endl;
        return;
    }

    memoryManager_->deallocate(ptr);
    std::cout << "Freed memory at " << ptr << std::endl;
}

int Runtime::readInput(char* buffer, size_t size) {
    if (!ioManager_) {
        std::cerr << "Error: I/O manager not initialized" << std::endl;
        return -1;
    }

    return ioManager_->readInput(buffer, size);
}

int Runtime::writeOutput(const char* buffer, size_t size) {
    if (!ioManager_) {
        std::cerr << "Error: I/O manager not initialized" << std::endl;
        return -1;
    }

    return ioManager_->writeOutput(buffer, size);
}

bool Runtime::parseExecutableFormat(const std::vector<char>& buffer) {
    // Simplified executable parsing - in real implementation, parse ELF/PE headers
    if (buffer.size() < 64) {
        std::cerr << "Error: Executable too small" << std::endl;
        return false;
    }

    // Check for ELF magic (0x7F 'E' 'L' 'F')
    if (buffer[0] == 0x7F && buffer[1] == 'E' && buffer[2] == 'L' && buffer[3] == 'F') {
        std::cout << "Detected ELF format executable" << std::endl;
        return parseELF(buffer);
    }

    // Check for PE magic ('M' 'Z')
    if (buffer[0] == 'M' && buffer[1] == 'Z') {
        std::cout << "Detected PE format executable" << std::endl;
        return parsePE(buffer);
    }

    std::cerr << "Error: Unknown executable format" << std::endl;
    return false;
}

bool Runtime::parseELF(const std::vector<char>& /*buffer*/) {
    // Simplified ELF parsing - extract entry point and sections
    // In real implementation, properly parse ELF header and sections

    // Assume entry point is at some offset (simplified)
    entryPoint_ = reinterpret_cast<void*>(0x400000);  // Typical ELF base address

    std::cout << "Parsed ELF executable, entry point: " << entryPoint_ << std::endl;
    return true;
}

bool Runtime::parsePE(const std::vector<char>& /*buffer*/) {
    // Simplified PE parsing - extract entry point and sections
    // In real implementation, properly parse PE header

    // Assume entry point is at some offset (simplified)
    entryPoint_ = reinterpret_cast<void*>(0x400000);  // Typical PE base address

    std::cout << "Parsed PE executable, entry point: " << entryPoint_ << std::endl;
    return true;
}

bool Runtime::setupExecutionEnvironment(int argc, char** argv) {
    if (!executionContext_) {
        std::cerr << "Error: Execution context not initialized" << std::endl;
        return false;
    }

    // Set up argument vector
    executionContext_->argc = argc;
    executionContext_->argv = argv;

    // Set up environment variables
    executionContext_->envp = environ;  // Standard environment

    // Initialize stack and heap
    executionContext_->stackBase = allocateMemory(1024 * 1024);  // 1MB stack
    executionContext_->heapBase = allocateMemory(1024 * 1024);   // 1MB heap

    if (!executionContext_->stackBase || !executionContext_->heapBase) {
        std::cerr << "Error: Failed to allocate stack or heap" << std::endl;
        return false;
    }

    std::cout << "Execution environment setup complete" << std::endl;
    return true;
}

int Runtime::executeProgram() {
    std::cout << "Starting program execution..." << std::endl;

    // In a real implementation, this would:
    // 1. Set up CPU state (registers, flags)
    // 2. Jump to entry point
    // 3. Handle system calls
    // 4. Manage memory and I/O

    // For simulation, just return success
    std::cout << "Program executed successfully (simulated)" << std::endl;
    return 0;
}

// IOManager implementation
int IOManager::readInput(char* buffer, size_t size) {
    if (inputBuffer_.empty()) {
        // Read from stdin
        std::string input;
        std::getline(std::cin, input);
        inputBuffer_.assign(input.begin(), input.end());
    }

    size_t copySize = std::min(size - 1, inputBuffer_.size());
    std::memcpy(buffer, inputBuffer_.data(), copySize);
    buffer[copySize] = '\0';

    // Remove the copied part from buffer
    inputBuffer_.erase(inputBuffer_.begin(), inputBuffer_.begin() + copySize);

    return static_cast<int>(copySize);
}

int IOManager::writeOutput(const char* buffer, size_t size) {
    std::cout.write(buffer, size);
    std::cout.flush();

    // Also store in output buffer for potential retrieval
    outputBuffer_.insert(outputBuffer_.end(), buffer, buffer + size);

    return static_cast<int>(size);
}

// ExecutionContext implementation
void ExecutionContext::cleanup() {
    // Cleanup any execution-specific resources
    // In a real implementation, this would handle stack/heap cleanup
}
