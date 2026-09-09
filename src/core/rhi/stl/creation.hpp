#pragma once
#include <cstddef>
// temporary
namespace Core::RHI {
  // Callbacks must be thread safe
  struct AllocationCallbacks {
    void * (*allocate)(void *userArg, size_t size, size_t alignment);
    void * (*reallocate)(void *userArg, void *memory, size_t size, size_t alignment);
    void (*free)(void *userArg, void *memory);
    void *userArg;
    bool disable3rdPartyAllocationCallbacks; // to use "AllocationCallbacks" only for NRI needs
  };
}