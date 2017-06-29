#if !defined(API_WIN32_MEMORY)
#include <windows.h>

static void*
Win32AllocateMemory(memory_index Size)
{
    // TODO(ray): Verify the type we are passing in make sure 
    //we are getting the proper page size back.
    void* Memory = VirtualAlloc(
        0,
        Size,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_READWRITE
        );
    if(!Memory)
    {
        // TODO(ray): Some error handling because we couldnt get the memory we need.
    }
    return Memory;
}

static void
Win32DeAllocateMemory(void* Memory,s64 Size)
{
    VirtualFree(
        Memory,
        Size,
        MEM_RELEASE
        );
}



#define API_WIN32_MEMORY
#endif