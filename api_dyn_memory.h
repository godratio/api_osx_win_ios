/*
author: Ray Garner 
email : raygarner13@gmail.com

api_memory  - public domain  emory allocing and deallocing - 
                                     no warranty implied; use at your own risk
                                     
                                     The Idea here is that you would these during development and release builds you would 
                                     use the static version by a compiler switch between dynamic and static memory types.
                                     This would allow you to have the benefits of dynamically growing arenas during development 
                                     but also give you the option to have the speed and stability of the static partion scheme if you needed it
                                     once you have locked down your memory limits.
                                     
                                     LICENSE
  See end of file for license information.
  
  */
//TODO(ray):Create API_STATIC_MEMORY_H
//with all static partitions
#if !defined(API_MEMORY_H)
#include "async.h"
#if OSX
#include <mach/mach_init.h>
#include <mach/mach_vm.h>

static void*
OSXAllocateMemory(memory_index Size)
{
    mach_vm_address_t address;
    kern_return_t kr;
    mach_vm_size_t size = (mach_vm_size_t)Size;
    kr = mach_vm_allocate(mach_task_self(), &address, size, VM_FLAGS_ANYWHERE);
    //TODO(ray):Make sure this actually casts properly.
    return (void*)address;
}

static void OSXDeAllocateMemory(void* Memory,s64 Size)
{
    mach_vm_address_t Address = (mach_vm_address_t)Memory;
    //mach_vm_size_t MachSize = (mach_vm_size_t)Size;
    mach_vm_deallocate(mach_task_self(), Address, Size);
}

#endif

#if IOS
#include <mach/mach_init.h>
//#include <mach/mach_vm.h>

static void*
IOSAllocateMemory(memory_index Size)
{
    mach_vm_address_t address;
    kern_return_t kr;
    mach_vm_size_t size = (mach_vm_size_t)Size;
    
    kr = _kernelrpc_mach_vm_allocate_trap(mach_task_self(), &address, size, VM_FLAGS_ANYWHERE);
    //TODO(ray):Make sure this actually casts properly.
    return (void*)address;
}

static void 
IOSDeAllocateMemory(void* Memory,s64 Size)
{
    mach_vm_address_t Address = (mach_vm_address_t)Memory;
    //mach_vm_size_t MachSize = (mach_vm_size_t)Size;
    //vm_deallocate(mach_task_self(), Address, Size);
    _kernelrpc_mach_vm_deallocate_trap(mach_task_self(), Address, Size);
}

#endif

#if WINDOWS
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
#endif

enum partition_push_flag
{
    PartitionFlag_ClearToZero = 0x1,
};

struct partition_push_params
{
    u32 Flags;
    u32 Alignment;
};

inline partition_push_params
DefaultPartitionParams(void)
{
    partition_push_params Params;
    Params.Flags = PartitionFlag_ClearToZero;
    Params.Alignment = 4;
    return(Params);
}

inline partition_push_params
AlignNoClear(u32 Alignment)
{
    partition_push_params Params = DefaultPartitionParams();
    Params.Flags &= ~PartitionFlag_ClearToZero;
    Params.Alignment = Alignment;
    return(Params);
}

inline partition_push_params
Align(u32 Alignment, b32 Clear)
{
    partition_push_params Params = DefaultPartitionParams();
    if(Clear)
    {
        Params.Flags |= PartitionFlag_ClearToZero;
    }
    else
    {
        Params.Flags &= ~PartitionFlag_ClearToZero;
    }
    Params.Alignment = Alignment;
    return(Params);
}

inline partition_push_params
NoClear(void)
{
    partition_push_params Params = DefaultPartitionParams();
    Params.Flags &= ~PartitionFlag_ClearToZero;
    return(Params);
}

struct partition_bootstrap_params
{
    u64 AllocationFlags;
    memory_index MinimumBlockSize;
};

inline partition_bootstrap_params
DefaultBootstrapParams(void)
{
    partition_bootstrap_params Params = {};
    return(Params);
}

enum platform_memory_block_flags
{
    PlatformMemory_NotRestored = 0x1,
    PlatformMemory_OverflowCheck = 0x2,
    PlatformMemory_UnderflowCheck = 0x4,
};

struct platform_memory_block
{
    u64 Flags;
    u64 Size;
    u8 *Base;
    memory_index Used;
    platform_memory_block *PartionPrev;
};

inline partition_bootstrap_params
NonRestoredArena(void)
{
    partition_bootstrap_params Params = DefaultBootstrapParams();
    Params.AllocationFlags = PlatformMemory_NotRestored;
    return(Params);
}

struct memory_partition
{
    // TODO(casey): If we see perf problems here, maybe move Used/Base/Size out?
    platform_memory_block *CurrentBlock;
    memory_index MinimumBlockSize;
    
    u64 AllocationFlags;
    s32 TempCount;
};

struct duel_memory_partition
{
    memory_partition *FixedSized;
    memory_partition *VariableSized;
};

struct temp_memory
{
    memory_partition *Arena;
    platform_memory_block *Block;
    memory_index Used;
};

inline void
SetMinimumBlockSize(memory_partition *Arena, memory_index MinimumBlockSize)
{
    Arena->MinimumBlockSize = MinimumBlockSize;
}

static void* 
GetPartitionPointer(memory_partition Partition)
{
    void* Result;
    Result = (uint8_t*)Partition.CurrentBlock->Base + Partition.CurrentBlock->Used;
    return Result;
}

struct memory_block
{
    platform_memory_block Block;
    memory_block *Prev;
    memory_block *Next;
    u64 LoopingFlags;
};

struct platform_state
{
    ticket_mutex MemoryMutex;
    memory_block MemorySentinel;
};

global_variable platform_state PlatformState;

inline void*
PlatformAllocateMemory(memory_index Size,u64 Flags = 0)
{
    void* Result;
    
    Assert(sizeof(memory_block) == 64);
    
    memory_index PageSize = 4096;
    memory_index TotalSize = Size + sizeof(memory_block);
    memory_index BaseOffset = sizeof(memory_block);
    memory_index ProtectOffset = 0;
    
    if(Flags & PlatformMemory_UnderflowCheck)
    {
        TotalSize = Size + 2*PageSize;
        BaseOffset = 2*PageSize;
        ProtectOffset = PageSize;
    }
    else if(Flags & PlatformMemory_OverflowCheck)
    {
        memory_index SizeRoundedUp = AlignPow2(Size, PageSize);
        TotalSize = SizeRoundedUp + 2*PageSize;
        BaseOffset = PageSize + SizeRoundedUp - Size;
        ProtectOffset = PageSize + SizeRoundedUp;
    }
    
    memory_block* Block; 
    
#if OSX
    Block = (memory_block *)OSXAllocateMemory(Size);
#elif WINDOWS
    Block = (memory_block *)Win32AllocateMemory(Size);
#elif IOS
    Block = (memory_block *)IOSAllocateMemory(Size);
#endif
    
    Assert(Block);
    Block->Block.Base = (u8 *)Block + BaseOffset;
    Assert(Block->Block.Used == 0);
    Assert(Block->Block.PartionPrev == 0);
    
    if(Flags & (PlatformMemory_UnderflowCheck|PlatformMemory_OverflowCheck))
    {
        DWORD OldProtect = 0;
        BOOL Protected = VirtualProtect((u8 *)Block + ProtectOffset, PageSize, PAGE_NOACCESS, &OldProtect);
        Assert(Protected);
    }
    
    memory_block *Sentinel = &PlatformState.MemorySentinel;
    Block->Next = Sentinel;
    Block->Block.Size = Size;
    Block->Block.Flags = Flags;
    
    BeginTicketMutex(&PlatformState.MemoryMutex);
    Block->Prev = Sentinel->Prev;
    Block->Prev->Next = Block;
    Block->Next->Prev = Block;
    EndTicketMutex(&PlatformState.MemoryMutex);
    
    platform_memory_block *PlatBlock = &Block->Block;
    
    return(PlatBlock);
}

inline void
PlatformDeAllocateMemory(void* Memory, memory_index Size)
{
    
#if OSX
    OSXDeAllocateMemory(Memory, Size);
#elif WINDOWS
    Win32DeAllocateMemory(Memory,Size);
#elif IOS
    IOSDeAllocateMemory(Memory,Size);
#endif
}

inline memory_index
GetAlignmentOffset(memory_partition *Arena, memory_index Alignment)
{
    memory_index AlignmentOffset = 0;
    
    memory_index ResultPointer = (memory_index)Arena->CurrentBlock->Base + Arena->CurrentBlock->Used;
    memory_index AlignmentMask = Alignment - 1;
    if(ResultPointer & AlignmentMask)
    {
        AlignmentOffset = Alignment - (ResultPointer & AlignmentMask);
    }
    
    return(AlignmentOffset);
}

inline temp_memory
BeginTempMemory(memory_partition *Partition)
{
    temp_memory Result;
    
    Result.Arena = Partition;
    Result.Block = Partition->CurrentBlock;
    Result.Used = Partition->CurrentBlock ? Partition->CurrentBlock->Used : 0;
    
    ++Partition->TempCount;
    
    return(Result);
}

inline void
FreeLastBlock(memory_partition *Arena)
{
    platform_memory_block *Free = Arena->CurrentBlock;
    Arena->CurrentBlock = Free->PartionPrev;
    PlatformDeAllocateMemory(Free,Free->PartionPrev->Size);
}

inline void
EndTemporaryMemory(temp_memory TempMem)
{
    memory_partition *Arena = TempMem.Arena;
    while(Arena->CurrentBlock != TempMem.Block)
    {
        FreeLastBlock(Arena);
    }
    
    if(Arena->CurrentBlock)
    {
        Assert(Arena->CurrentBlock->Used >= TempMem.Used);
        Arena->CurrentBlock->Used = TempMem.Used;
        Assert(Arena->TempCount > 0);
    }
    --Arena->TempCount;
}

static void ValidateTempMemory(temp_memory TempMem)
{
    Assert(TempMem.Arena->TempCount < 1);
}

static b32 TestFlag(u32 Flag, u32 TestAgaist)
{
    if ((Flag & TestAgaist) == TestAgaist)
    {
        return true;
    }
    return false;
}

#define ZeroStruct(Instance) ClearToZero(&(Instance),sizeof(Instance))
#define ZeroArray(Count, Pointer) ClearToZero(Pointer,Count*sizeof((Pointer)[0]))
static void ClearToZero(void* Mem,u32 Size)
{
    Assert(Size > 0)
        u8* Byte = (u8*)Mem;
    while (Size--)
    {
        *Byte++ = 0;
    }
}

#define PushArray(Partition,Type,Count,...) PushSize_(Partition,sizeof(Type)*Count,## __VA_ARGS__)
#define PushStruct(Partition,Type,...) (Type*)PushSize_(Partition,sizeof(Type),## __VA_ARGS__)
#define PushSize(Partition,Size,...) PushSize_(Partition,Size,## __VA_ARGS__)
#define PushCopy(Partition, Size, Source, ...) Copy(Size, Source, PushSize_(Partition, Size, ## __VA_ARGS__))

inline memory_index
GetEffectiveSizeFor(memory_partition *Arena, memory_index SizeInit, partition_push_params Params = DefaultPartitionParams())
{
    memory_index Size = SizeInit;
    
    memory_index AlignmentOffset = GetAlignmentOffset(Arena, Params.Alignment);
    Size += AlignmentOffset;
    
    return(Size);
}
#define Minimum(A, B) ((A < B) ? (A) : (B))
#define Maximum(A, B) ((A > B) ? (A) : (B))

inline void *
PushSize_(memory_partition *Partition, memory_index SizeInit, partition_push_params Params = DefaultPartitionParams())
{
    void *Result = 0;
    
    memory_index Size = 0;
    if(Partition->CurrentBlock)
    {
        Size = GetEffectiveSizeFor(Partition, SizeInit, Params);
    }
    
    if(!Partition->CurrentBlock ||
       (Partition->CurrentBlock->Used + Size) > Partition->CurrentBlock->Size)
    {
        Size = SizeInit; 
        if(Partition->AllocationFlags & (PlatformMemory_OverflowCheck|
                                         PlatformMemory_UnderflowCheck))
        {
            Partition->MinimumBlockSize = 0;
            Size = AlignPow2(Size, Params.Alignment);
        }
        else if(!Partition->MinimumBlockSize)
        {
            Partition->MinimumBlockSize = 1024*1024;
        }
        
        memory_index BlockSize = Maximum(Size, Partition->MinimumBlockSize);
        
        platform_memory_block *NewBlock = 
            (platform_memory_block*)PlatformAllocateMemory(BlockSize, Partition->AllocationFlags);
        NewBlock->PartionPrev = Partition->CurrentBlock;
        Partition->CurrentBlock = NewBlock;
    }    
    
    Assert((Partition->CurrentBlock->Used + Size) <= Partition->CurrentBlock->Size);
    
    memory_index AlignmentOffset = GetAlignmentOffset(Partition, Params.Alignment);
    Result = Partition->CurrentBlock->Base + Partition->CurrentBlock->Used + AlignmentOffset;
    Partition->CurrentBlock->Used += Size;
    
    Assert(Size >= SizeInit);
    
    if(Params.Flags & PartitionFlag_ClearToZero)
    {
        ClearToZero(Result,(u32)SizeInit);
    }
    
    return(Result);
}
/*
static void* PushSize_(memory_partition*Partition, u32 Size,partition_push_params PushParams = DefaultPartitionParams())
{
    //Assert Used < Size
    Assert(Partition->Used + Size <= Partition->Size)
        void* Result;
    Result = (uint8_t*)Partition->Base + Partition->Used;
    if (TestFlag(PushParams.Flags, PartitionFlag_ClearToZero))
    {
        ClearSize(Partition, Size);
    }
    Partition->Used = Partition->Used + Size;
    
    return Result;
}
*/

inline char *
PushCharString(memory_partition *Partition, char *CharString)
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

inline void
CheckArena(memory_partition *Arena)
{
    Assert(Arena->TempCount == 0);
}

inline void *
Copy(memory_index Size, void *SourceInit, void *DestInit)
{
    u8 *Source = (u8 *)SourceInit;
    u8 *Dest = (u8 *)DestInit;
    while(Size--) {*Dest++ = *Source++;}
    
    return(DestInit);
}

#define BootstrapPushStruct(type, Member, ...) (type *)BootstrapPushSize_(sizeof(type), OffsetOf(type, Member), ## __VA_ARGS__)
inline void *
BootstrapPushSize_(memory_index StructSize, memory_index OffsetToArena,
                   partition_bootstrap_params BootstrapParams = DefaultBootstrapParams(), 
                   partition_push_params Params = DefaultPartitionParams())
{
    memory_partition Bootstrap = {};
    Bootstrap.AllocationFlags = BootstrapParams.AllocationFlags;
    Bootstrap.MinimumBlockSize = BootstrapParams.MinimumBlockSize;
    void *Struct = PushSize(&Bootstrap, StructSize, Params);
    *(memory_partition *)((u8 *)Struct + OffsetToArena) = Bootstrap;
    return(Struct);
}

#define API_MEMORY_H
#endif


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
