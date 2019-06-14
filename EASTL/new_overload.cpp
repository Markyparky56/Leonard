#include "EASTL\allocator.h"

void* operator new[](size_t size, const char* name, int flags, unsigned debugFlags, const char* file, int line)
{
  return _aligned_malloc(size, EA_PLATFORM_MIN_MALLOC_ALIGNMENT);
}

void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
  return _aligned_offset_malloc(size, alignment, alignmentOffset);
}
