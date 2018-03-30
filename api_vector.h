/*
author: Ray Garner 
email : raygarner13@gmail.com

api_vector  - public domain vector(not vectorized yet) fixed size only 
but size can grow up with a pre alloced black of memory - 
                                     no warranty implied; use at your own risk
                                     
                                     
                                     LICENSE
  See end of file for license information.
  
  */

#if !defined(API_VECTOR_H)
#include <stdint.h>

#include "api_memory.h"
typedef uint32_t u32;
typedef uint32_t b32;

struct vector
{
    void* Base;
    u32 TotalSize;
    u32 UnitSize;
    u32 Count;
    u32 TotalCount;
    u32 AtIndex;
    s32 StartAt;
    u32** FreeList;
    MemoryArena* Partition;
    u32 Pushable;
};

//TODO(ray):Add memory alignment options here
static vector CreateVector(u32 StartSize,u32 UnitSize,b32 PreEmpt = false)
{
    //TIMED_BLOCK();
    Assert(StartSize > 0);
    Assert(UnitSize > 0);
    
    vector Result;
    
    Result.TotalSize = StartSize * UnitSize;
    Result.UnitSize = UnitSize;
    Result.TotalCount = StartSize;
    
    
    Result.AtIndex = 0;
    Result.StartAt = -1;
    Result.Pushable = true;
    //TODO(ray): change this to get memory froma a pre allocated partition.
    void* StartingMemory = PlatformAllocateMemory(Result.TotalSize);
    MemoryArena* Partition = (MemoryArena*)StartingMemory;
    AllocatePartition(Partition, Result.TotalSize,Partition+sizeof(MemoryArena*));
    
    Result.Partition = Partition;
    if(PreEmpt)
    {
        Result.Count = StartSize;
        PushSize_(Partition,Result.TotalSize);
    }
    else
    {
        Result.Count = 0;
    }
    
    
    Result.Base = Partition->Base;
    
    
    return Result;
}

#define GetVectorElement(Type,Vector,Index) (Type*)GetVectorElement_(Vector,Index)
#define GetVectorFirst(Type,Vector) (Type*)GetVectorElement_(Vector,0)
#define GetVectorLast(Type,Vector) (Type*)GetVectorElement_(Vector,Vector.Count)
#define PeekVectorElement(Type,Vector) (Type*)GetVectorElement_(Vector,*Vector.Count-1)

static void* GetVectorElement_(vector* Vector, u32 Index)
{
    //TIMED_BLOCK();
    Assert(Vector);
    
    //TODO(Ray):May want to think about this. Need to give a hint to the client code.
    //Get and PushCast are almost the same.
    //Assert(Vector->Count >= Index + 1);
    void* Location = (uint8_t*)Vector->Base + (Index * Vector->UnitSize);
    return Location;
}

static void* SetVectorElement(vector* Vector, u32 ElementIndex,void* Element,b32 Copy = true)
{
    //TIMED_BLOCK();
    Assert(Vector && Element);
    Vector->Pushable = false;
    //TODO(ray):have some protection here to make sure we are added in the right type.
    void* Location = (uint8_t*)Vector->Base + (ElementIndex * Vector->UnitSize);
    uint8_t* Ptr = (uint8_t*)Location;
    if (Copy)
    {
        u32 ByteCount = Vector->UnitSize;
        u32 Index = 0;
        while (Index < ByteCount)
        {
            *Ptr++ = *((uint8_t*)Element + Index);
            Index++;
        }
    }
    else
    {
        Ptr = (uint8_t*)Element;
    }
    
    Vector->TotalSize += Vector->UnitSize;
    //TODO(Ray):This does not make much sense was added for the vertex and normal
    //setting in Mesh importer should probably not do this here and think aobut it
    //for now its fine.
    Vector->Count++;
    return Location;
}

//NOTE(ray):If you use SetVectorElement Pushes will no longer work properly.
static u32 PushVectorElement(vector* Vector, void* Element, b32 Copy = true)
{
    //TIMED_BLOCK();
    Assert(Vector && Element);
    Assert(Vector->Pushable);
    
    //TODO(ray):have some protection here to make sure we are added in the right type.
    uint8_t *Ptr = (uint8_t*)PushSize(Vector->Partition, Vector->UnitSize);
    if(!Ptr)
    {
        u32 NewMemSize = Vector->TotalSize*2;
        //Get more mem
        Ptr = (u8*)PlatformAllocateMemory(NewMemSize);
        memcpy(Vector->Partition->Base,Ptr,NewMemSize);
        Ptr = Ptr + Vector->TotalSize;
    }
    if (Copy)
    {
        u32 ByteCount = Vector->UnitSize;
        u32 Index = 0;
        while (Index < ByteCount)
        {
            *Ptr++ = *((uint8_t*)Element + Index);
            Index++;
        }
    }
    else
    {
        Ptr = (uint8_t*)Element;
    }
    Vector->TotalSize += Vector->UnitSize;
    u32 ResultIndex = Vector->Count++;
    return ResultIndex;
}

#define PushEmptyVectorElement(Vector) PushEmptyVectorElement_(Vector)
#define PushAndCastEmptyVectorElement(Type,Vector) (Type*)PushEmptyVectorElement_(Vector)
static void* PushEmptyVectorElement_(vector* Vector)
{
    //TIMED_BLOCK();
    //TODO(ray):have some protection here to make sure we are added in the right type.
    uint8_t *Ptr = (uint8_t*)PushSize(Vector->Partition, Vector->UnitSize);
    
    Vector->TotalSize += Vector->UnitSize;
    Vector->Count++;
    return (void*)Ptr;
}

#define PopAndPeekVectorElement(Type,Vector) (Type*)PopAndPeekVectorElement_(Vector)
static void* PopAndPeekVectorElement_(vector* Vector)
{
    //TIMED_BLOCK();
    Assert(Vector);
    void* Result = GetVectorElement_(Vector,Vector->Count);
    Vector->Partition->Used -= Vector->UnitSize;
    Vector->TotalSize -= Vector->UnitSize;
    Vector->Count--;
    return Result;
}

static void PopVectorElement(vector* Vector)
{
    //TIMED_BLOCK();
    Assert(Vector);
    Vector->Partition->Used -= Vector->UnitSize;
    Vector->TotalSize -= Vector->UnitSize;
    Vector->Count--;
}

#define IterateVector(Vector,Type) (Type*)IterateVectorElement_(Vector)
#define IterateVectorFromIndex(Vector,Type,Index) (Type*)IterateVectorElement_(Vector,Index)
static void* IterateVectorElement_(vector *Vector, s32 StartAt = -1)
{
    //TIMED_BLOCK();
    Assert(Vector);
    if (Vector->AtIndex >= Vector->Count)return 0;
    if (StartAt >= 0 && Vector->StartAt < 0)
    {
        Vector->AtIndex = StartAt;
        Vector->StartAt = StartAt;
    }
    else
    {
        
    }
    return GetVectorElement_(Vector, Vector->AtIndex++);
}

static void ResetVectorIterator(vector *Vector)
{
    //TIMED_BLOCK();
    Assert(Vector);
    Vector->AtIndex = 0;
    Vector->StartAt = -1;
}

static void ClearVector(vector *Vector)
{
    //TIMED_BLOCK();
    Assert(Vector);
    Vector->Partition->Used = 0;
    Vector->Count = 0;
    Vector->AtIndex = 0;
    Vector->StartAt = -1;
    Vector->Partition->TempCount = 0;
}

static void FreeVectorMem(vector *Vector)
{
    //TIMED_BLOCK();
    if(Vector->TotalSize > 0)
    {
        ClearVector(Vector);
        PlatformDeAllocateMemory(Vector->Partition->Base,Vector->Partition->Size);
        
    }
}

#define API_VECTOR_H
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
