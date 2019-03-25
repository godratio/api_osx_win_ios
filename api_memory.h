#if !defined(API_MEMORY_H)
/*
author: Ray Garner 
email : raygarner13@gmail.com

api_vector  - public domain  emory allocing and deallocing - 
                                     no warranty implied; use at your own risk
                                     
                                     LICENSE
  See end of file for license information.
  
  */

enum partition_push_flag
{
    PartitionFlag_None = 0x0,
    PartitionFlag_ClearToZero = 0x1,
};

struct partition_push_params
{
    u32 Flags;
    u32 Alignment;
};

struct MemoryArena
{
    void* base;
    memory_index size;
    uint64_t used;
    uint64_t temp_count;
};

struct temp_memory
{
    MemoryArena *Partition;
    u32 used;
};

struct duel_memory_partition
{
    MemoryArena FixedSized;
    MemoryArena VariableSized;
};

#if OSX
#include <mach/mach_init.h>
#include <mach/mach_vm.h>

static void* OSXAllocateMemory(memory_index in_size);
static void OSXDeAllocateMemory(void* Memory,s64 size);
#endif

#if IOS
#include <mach/mach_init.h>
static void* IOSAllocateMemory(memory_index in_size);
static void IOSDeAllocateMemory(void* Memory,s64 size);
#endif

#if WINDOWS
#include <windows.h>
static void* Win32AllocateMemory(memory_index Size);
static void Win32DeAllocateMemory(void* Memory,s64 Size);
#endif

inline partition_push_params DefaultPartitionParams();

void* GetPartitionPointer(MemoryArena Partition);

void* PlatformAllocateMemory(memory_index in_size);

void PlatformDeAllocateMemory(void* Memory, memory_index in_size);

inline memory_index GetAlignmentOffset(MemoryArena *Arena, memory_index Alignment);
 
 void FreeDuelMemoryPartion(duel_memory_partition* Partition);

 void FreeMemoryPartion(MemoryArena* Partition);

 MemoryArena AllocateMemoryPartition(u32 Size);

 duel_memory_partition AllocateDuelMemoryPartition(u32 Size);

inline void AllocatePartition(MemoryArena *Partition, memory_index Size, void* Base);

inline MemoryArena AllocatePartition(memory_index Size, void* Base);

inline MemoryArena PlatformAllocatePartition(memory_index Size);

//NOTE(Ray):REmvoed because dumb. wrote this when I was still a noob to pointers and c.
//inline MemoryArena* PlatformAllocatePartition(memory_index Size);

//TODO(ray):Fix this to clear more effeciently. or give option for clearing  method
 void ClearToZero(MemoryArena *Partition);

 void DeAllocatePartition(MemoryArena *Partition, bool ClearMemToZero = true);

 temp_memory BeginTempMemory(MemoryArena *Partition);

 void EndTempMemory(temp_memory TempMem);

 void ValidateTempMemory(temp_memory TempMem);

 b32 TestFlag(u32 Flag, u32 TestAgaist);

 void ClearSize(MemoryArena *Partition,u32 Size);

#define ZeroStruct(Instance) ClearToZero(&(Instance),sizeof(Instance))
#define ZeroArray(Count, Pointer) ClearToZero(Pointer,Count*sizeof((Pointer)[0]))
 void ClearToZero(void* Mem,u32 Size);

#define PushArray(Partition,Type,Count,...) (Type*)PushSize_(Partition,sizeof(Type)*Count,## __VA_ARGS__)
#define PushStruct(Partition,Type,...) (Type*)PushSize_(Partition,sizeof(Type),## __VA_ARGS__)
#define PushSize(Partition,Size,...) PushSize_(Partition,Size,## __VA_ARGS__)
 void* PushSize_(MemoryArena*Partition,uint64_t Size,partition_push_params PushParams = DefaultPartitionParams());
inline partition_push_params NoClear();
inline char *PushCharString(MemoryArena *Partition, char *CharString);

#define PushArrayA(Partition,Type,Count,...) (Type*)PushSize_(Partition,sizeof(Type)*Count,## __VA_ARGS__)
#define PushStructA(Partition,Type,...) (Type*)PushSizeA_(Partition,sizeof(Type),## __VA_ARGS__)
#define PushSizeA(Partition,Size,...) PushSizeA_(Partition,Size,## __VA_ARGS__)

void* PushSizeA_(MemoryArena* arena,memory_index size,partition_push_params params);
    
#if OSX
#include <mach/mach_init.h>
#include <mach/mach_vm.h>

void* OSXAllocateMemory(memory_index in_size)
{
    mach_vm_address_t address;
    kern_return_t kr;
    mach_vm_size_t size = (mach_vm_size_t)in_size;
    kr = mach_vm_allocate(mach_task_self(), &address, size, VM_FLAGS_ANYWHERE);
    //TODO(ray):Make sure this actually casts properly.
    return (void*)address;
}

void OSXDeAllocateMemory(void* Memory,s64 size)
{
    mach_vm_address_t Address = (mach_vm_address_t)Memory;
    //mach_vm_size_t MachSize = (mach_vm_size_t)size;
    mach_vm_deallocate(mach_task_self(), Address, size);
}
#endif

#if IOS
#include <mach/mach_init.h>
void* IOSAllocateMemory(memory_index in_size)
{
    mach_vm_address_t address;
    kern_return_t kr;
    mach_vm_size_t size = (mach_vm_size_t)in_size;
    kr = _kernelrpc_mach_vm_allocate_trap(mach_task_self(), &address, size, VM_FLAGS_ANYWHERE);
    //TODO(ray):Make sure this actually casts properly.
    return (void*)address;
}

void IOSDeAllocateMemory(void* Memory,s64 size)
{
    mach_vm_address_t Address = (mach_vm_address_t)Memory;
    //mach_vm_size_t MachSize = (mach_vm_size_t)size;
    //vm_deallocate(mach_task_self(), Address, size);
    _kernelrpc_mach_vm_deallocate_trap(mach_task_self(), Address, size);
}
#endif

#if WINDOWS
#include <windows.h>
void* Win32AllocateMemory(memory_index Size)
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

void Win32DeAllocateMemory(void* Memory,s64 Size)
{
    VirtualFree(
        Memory,
        Size,
        MEM_RELEASE
        );
}
#endif

inline partition_push_params DefaultPartitionParams()
{
    partition_push_params Params;
    Params.Flags = PartitionFlag_ClearToZero;
    Params.Alignment = 4;
    return(Params);
}

inline void* PlatformAllocateMemory(memory_index in_size)
{
    void* Result;
#if OSX
    Result = OSXAllocateMemory(in_size);
#elif WINDOWS
    Result = Win32AllocateMemory(in_size);
#elif IOS
    Result = IOSAllocateMemory(in_size);
#endif
    return Result;
}

inline void PlatformDeAllocateMemory(void* Memory, memory_index in_size)
{
#if OSX
    OSXDeAllocateMemory(Memory, in_size);
#elif WINDOWS
    //TODO(Ray):Verify windows is really freeing this memory.
    Win32DeAllocateMemory(Memory,in_size);
#elif IOS
    IOSDeAllocateMemory(Memory,in_size);
#endif
}

inline memory_index GetAlignmentOffset(MemoryArena *Arena, memory_index Alignment)
{
    if(Alignment == 0)return 0;
    memory_index AlignmentOffset = 0;
    memory_index ResultPointer = (memory_index)Arena->base + Arena->used;
    memory_index AlignmentMask = Alignment - 1;
    if(ResultPointer & AlignmentMask)
    {
        AlignmentOffset = Alignment - (ResultPointer & AlignmentMask);
    }
    return(AlignmentOffset);
}

inline void AllocatePartition(MemoryArena *Partition, memory_index Size, void* Base)
{
    //Assert
    Partition->base = Base;
    Partition->size = Size;
    Partition->used = 0;
    Partition->temp_count = 0;
}

inline MemoryArena AllocatePartition(memory_index Size, void* Base)
{
    MemoryArena result;
    //Assert
    result.base = Base;
    result.size = Size;
    result.used = 0;
    result.temp_count = 0;
    return result;
}

inline MemoryArena PlatformAllocatePartition(memory_index size)
{
    u8* mem = (u8*)PlatformAllocateMemory(size);
    MemoryArena arena = {};
    AllocatePartition(&arena,size,mem);
    return arena;
}

/*
//TODO(Ray):Remove this garbage causes a small memory leak due to when you release
inline MemoryArena* PlatformAllocatePartition(memory_index Size)
{
    memory_index SizeOfMP = sizeof(MemoryArena);
    Size += SizeOfMP;
    u8* Mem = (u8*)PlatformAllocateMemory(Size);
    MemoryArena* Header = (MemoryArena *)Mem;
    AllocatePartition(Header,Size - SizeOfMP, Mem + SizeOfMP);
    return Header;
}
*/

#ifdef YOYOIMPL

void* GetPartitionPointer(MemoryArena Partition)
{
    return (uint8_t*)Partition.base + Partition.used;
}
 
void FreeDuelMemoryPartion(duel_memory_partition* Partition)
{
    PlatformDeAllocateMemory(Partition->FixedSized.base,Partition->FixedSized.size);    
    PlatformDeAllocateMemory(Partition->VariableSized.base,Partition->VariableSized.size);    
}

void FreeMemoryPartion(MemoryArena* Partition)
{
    PlatformDeAllocateMemory(Partition->base,Partition->size);    
}

MemoryArena AllocateMemoryPartition(u32 Size)
{
    MemoryArena Result;
    void* Base = PlatformAllocateMemory(Size);
    Result.base = Base;
    Result.size = Size;
    Result.used = 0;
    Result.temp_count = 0;
    return Result;    
}

duel_memory_partition AllocateDuelMemoryPartition(u32 Size)
{
    duel_memory_partition Result;
    Result.FixedSized = AllocateMemoryPartition(Size);
    Result.VariableSized = AllocateMemoryPartition(Size);
    return Result;
}

//TODO(ray):Fix this to clear more effeciently. or give option for clearing  method
void ClearToZero(MemoryArena *Partition)
{
    for (u32 ByteIndex = 0; ByteIndex < Partition->size; ++ByteIndex)
    {
        uint8_t* Byte = (uint8_t*)Partition->base + ByteIndex;
        *Byte = 0;
    }
}

void DeAllocatePartition(MemoryArena *Partition, bool ClearMemToZero)
{
    //Assert
    Partition->used = 0;
    if (ClearMemToZero)
    {
        ClearToZero(Partition);
    }
}

MemoryArena ResizeArena(MemoryArena* partition,memory_index new_size)
{
    MemoryArena new_arena = AllocateMemoryPartition(new_size);
    memcpy(new_arena.base,partition->base,partition->used);
    DeAllocatePartition(partition,false);
    FreeMemoryPartion(partition);
    return new_arena;
}

temp_memory BeginTempMemory(MemoryArena *Partition)
{
    temp_memory Result;
    Result.Partition = Partition;
    Partition->temp_count++;
    Result.used = Partition->used;
    return Result;
}

void EndTempMemory(temp_memory TempMem)
{
    TempMem.Partition->used = TempMem.used;
    TempMem.Partition->temp_count--;
}

void ValidateTempMemory(temp_memory TempMem)
{
    Assert(TempMem.Partition->temp_count < 1);
}

b32 TestFlag(u32 Flag, u32 TestAgaist)
{
    if ((Flag & TestAgaist) == TestAgaist)
    {
        return true;
    }
    return false;
}

void ClearSize(MemoryArena *Partition,u32 Size)
{
    Assert(Size > 0)
        Size--;
    if(Partition->used < Size)
    {
        Size = Partition->used;
    }
    
    if(Partition->used == 0)
    {
        return;
    }
    else
    {
        u8* Byte = (u8*)Partition->base + Partition->used;
        while (Size--)
        {
            *Byte++ = 0;
        }
    }
}

void ClearToZero(void* Mem,u32 Size)
{
    Assert(Size > 0)
        u8* Byte = (u8*)Mem;
    while (Size--)
    {
        *Byte++ = 0;
    }
}

void* PushSize_(MemoryArena*Partition,uint64_t Size,partition_push_params PushParams)
{
    Assert(Partition->used + Size <= Partition->size)
    if (TestFlag(PushParams.Flags, PartitionFlag_ClearToZero))
    {
        ClearSize(Partition, Size);
    }
    void* result = (uint8_t*)Partition->base + Partition->used;
    Partition->used = Partition->used + Size;
    return result;
}

//aligned only allocator
void* PushSizeA_(MemoryArena* arena,memory_index size,partition_push_params params)
{
    memory_index offset = GetAlignmentOffset(arena,params.Alignment);
    Assert(arena->used + (size + offset) <= arena->size);
    if (TestFlag(params.Flags, PartitionFlag_ClearToZero))
    {
        ClearSize(arena, size);
    }
    
    void* result = (void*)((uint8_t*)arena->base + (arena->used + offset));
    //size += offset;
    arena->used = arena->used + (size + offset);
    return result;
}

inline partition_push_params NoClear()
{
    partition_push_params Params = DefaultPartitionParams();
    Params.Flags &= ~PartitionFlag_ClearToZero;
    return(Params);
}

inline char *PushCharString(MemoryArena *Partition, char *CharString)
{
    u32 Size = 1;
    for(char *At = CharString;
        *At;
        ++At)
    {
        ++Size;
    }
    
    char *Dest = (char *)PushSize_(Partition, Size, NoClear());
    for(u32 CharIndex = 0;
        CharIndex < Size;
        ++CharIndex)
    {
        Dest[CharIndex] = CharString[CharIndex];
    }
    
    return(Dest);
}


/*
------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2017 Sean Barrett
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
------------------------------------------------------------------------------
ALTERNATIVE B - Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
------------------------------------------------------------------------------
*/
#endif

#define API_MEMORY_H
#endif
