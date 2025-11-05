#include "Runtime.h"

// Placeholder MemoryManager implementation
class MemoryManager {
public:
    MemoryManager() = default;
    ~MemoryManager() = default;

    void* allocate(size_t size) {
        return malloc(size);
    }

    void deallocate(void* ptr) {
        free(ptr);
    }

    void* reallocate(void* ptr, size_t newSize) {
        return realloc(ptr, newSize);
    }

    size_t getAllocatedSize() const {
        // TODO: Track actual allocated memory
        return 0;
    }
};
