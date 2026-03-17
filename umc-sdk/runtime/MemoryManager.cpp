#include "Runtime.h"

// MemoryManager implementation
MemoryManager::MemoryManager() = default;
MemoryManager::~MemoryManager() = default;

void* MemoryManager::allocate(size_t size) {
    void* ptr = malloc(size);
    if (ptr) {
        allocations_[ptr] = size;
    }
    return ptr;
}

void MemoryManager::deallocate(void* ptr) {
    auto it = allocations_.find(ptr);
    if (it != allocations_.end()) {
        allocations_.erase(it);
        free(ptr);
    }
}

void* MemoryManager::reallocate(void* ptr, size_t newSize) {
    auto it = allocations_.find(ptr);
    if (it != allocations_.end()) {
        allocations_.erase(it);
        void* newPtr = realloc(ptr, newSize);
        if (newPtr) {
            allocations_[newPtr] = newSize;
        }
        return newPtr;
    }
    return nullptr;
}

size_t MemoryManager::getAllocatedSize() const {
    size_t total = 0;
    for (const auto& alloc : allocations_) {
        total += alloc.second;
    }
    return total;
}
