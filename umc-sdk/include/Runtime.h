#ifndef RUNTIME_H
#define RUNTIME_H

#include <string>
#include <vector>
#include <map>

// Forward declarations for runtime components
class MemoryManager;
class IOManager;
class ExecutionContext;

class Runtime {
public:
    Runtime();
    ~Runtime();

    bool loadExecutable(const std::string& executablePath);
    bool execute(int argc, char** argv);
    void cleanup();

    // Runtime services
    void* allocateMemory(size_t size);
    void freeMemory(void* ptr);
    int readInput(char* buffer, size_t size);
    int writeOutput(const char* buffer, size_t size);

private:
    std::string executablePath_;
    void* loadedModule_;
    void* entryPoint_;

    MemoryManager* memoryManager_;
    IOManager* ioManager_;
    ExecutionContext* executionContext_;

    bool initializeRuntime();
    void shutdownRuntime();

    bool parseExecutableFormat(const std::vector<char>& buffer);
    bool parseELF(const std::vector<char>& buffer);
    bool parsePE(const std::vector<char>& buffer);

    bool setupExecutionEnvironment(int argc, char** argv);
    int executeProgram();
};

// Runtime component implementations
class MemoryManager {
public:
    MemoryManager() = default;
    ~MemoryManager() = default;

    void* allocate(size_t size);
    void deallocate(void* ptr);
    void* reallocate(void* ptr, size_t newSize);
    size_t getAllocatedSize() const;

private:
    std::map<void*, size_t> allocations_;
};

class IOManager {
public:
    IOManager() = default;
    ~IOManager() = default;

    int readInput(char* buffer, size_t size);
    int writeOutput(const char* buffer, size_t size);

private:
    std::vector<char> inputBuffer_;
    std::vector<char> outputBuffer_;
};

class ExecutionContext {
public:
    ExecutionContext() = default;
    ~ExecutionContext() = default;

    void cleanup();

    int argc;
    char** argv;
    char** envp;
    void* stackBase;
    void* heapBase;
};

#endif // RUNTIME_H
