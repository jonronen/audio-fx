#include <utils/heap.h>
#include <stdint.h>

void* operator new(unsigned int size) throw()
{
    return heap_alloc(size);
}

void operator delete(void* ptr) throw()
{
    heap_free(ptr);
}

