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

struct YoyoVector
{
    void* base;
	uint32_t total_size;
	uint32_t max_size;
	uint32_t unit_size;
	uint32_t count;
	uint32_t total_count;
	uint32_t at_index;
	int32_t start_at;
	uint32_t** free_list;
    MemoryArena* mem_arena;
    bool pushable;
	float resize_ratio = 0.5f;//0.1 10% 1 100% default is 50% or 1/2 resizing
	bool allow_resize = true;
};

//TODO(ray):Add memory alignment options here
static YoyoVector YoyoInitVector(u32 start_size, u32 unit_size, bool pre_empt = false)
{
    //TIMED_BLOCK();
    Assert(start_size > 0);
    Assert(unit_size > 0);
    
    YoyoVector result;
	result.total_size = 0;
	result.max_size = start_size * unit_size;
    result.unit_size = unit_size;
    result.total_count = start_size;
    
    result.at_index = 0;
    result.start_at = -1;
    result.pushable = true;
    //TODO(ray): change this to get memory froma a pre allocated partition.
    void* starting_memory = PlatformAllocateMemory(result.max_size);
    MemoryArena* partition = (MemoryArena*)starting_memory;
    AllocatePartition(partition, result.max_size,partition+sizeof(MemoryArena*));
    
    result.mem_arena = partition;
    if(pre_empt)
    {
        result.count = start_size;
        PushSize_(partition,result.max_size);
    }
    else
    {
        result.count = 0;
    }
    result.base = partition->Base;
    return result;
}

#define GetVectorElement(Type,Vector,Index) (Type*)GetVectorElement_(Vector,Index)
#define GetVectorFirst(Type,Vector) (Type*)GetVectorElement_(Vector,0)
#define GetVectorLast(Type,Vector) (Type*)GetVectorElement_(Vector,Vector.Count)
#define PeekVectorElement(Type,Vector) (Type*)GetVectorElement_(Vector,*Vector.Count-1)
static void* GetVectorElement_(YoyoVector* Vector, u32 Index)
{
    //TIMED_BLOCK();
    Assert(Vector);
    
    //TODO(Ray):May want to think about this. Need to give a hint to the client code.
    //Get and PushCast are almost the same.
    //Assert(Vector->Count >= Index + 1);
    void* Location = (uint8_t*)Vector->base + (Index * Vector->unit_size);
    return Location;
}

static void* SetVectorElement(YoyoVector* Vector, u32 ElementIndex, void* Element, bool Copy = true)
{
    //TIMED_BLOCK();
    Assert(Vector && Element);
    Vector->pushable = false;
    //TODO(ray):have some protection here to make sure we are added in the right type.
    void* Location = (uint8_t*)Vector->base + (ElementIndex * Vector->unit_size);
    uint8_t* Ptr = (uint8_t*)Location;
    if (Copy)
    {
        u32 ByteCount = Vector->unit_size;
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
    
    Vector->total_size += Vector->unit_size;
    //TODO(Ray):This does not make much sense was added for the vertex and normal
    //setting in Mesh importer should probably not do this here and think aobut it
    //for now its fine.
    Vector->count++;
    return Location;
}

//NOTE(ray):If you use SetVectorElement Pushes will no longer work properly.
/**
 * \brief Push an element of a vector.  
 * \param vector Point to the vector to push to 
 * \param element One element of the type you registered when creating the vector 
 * \param copy Should we do a byte for byte copy?  
 * \return returns an index into the array in the vector for the created element
 */
static u32 YoyoPushBack(YoyoVector* vector, void* element, bool copy = true)
{
    //TIMED_BLOCK();
    Assert(vector && element);
    Assert(vector->pushable);
	Assert(vector->start_at == -1);//You must have forget to reset the vector or are trying to resize during iteration.

    //TODO(Ray):Test this.
    //check if we have space if not resize to create it.
    if(vector->total_size < vector->unit_size * vector->count + 1)
    {
		u32 new_size = vector->total_size + (vector->total_size * vector->resize_ratio);
		u8* temp_ptr = (u8*)vector->base;
		vector->base = PlatformAllocateMemory(new_size);
		memcpy(vector->base, (void*)temp_ptr, vector->total_size);
		vector->total_size = new_size;
		free(temp_ptr);
    }

    //TODO(ray):have some protection here to make sure we are added in the right type.
    uint8_t *ptr = (uint8_t*)PushSize(vector->mem_arena, vector->unit_size);
    if (copy)
    {
        uint32_t byte_count = vector->unit_size;
        uint32_t index = 0;
        while (index < byte_count)
        {
            *ptr++ = *((uint8_t*)element + index);
            index++;
        }
    }
    else
    {
        ptr = (uint8_t*)element;
    }
    
    vector->total_size += vector->unit_size;
    u32 result_index = vector->count++;
    return result_index;
}

#define PushEmptyVectorElement(Vector) PushEmptyVectorElement_(Vector)
#define PushAndCastEmptyVectorElement(Type,Vector) (Type*)PushEmptyVectorElement_(Vector)
static void* PushEmptyVectorElement_(YoyoVector* Vector)
{
    //TIMED_BLOCK();
    //TODO(ray):have some protection here to make sure we are added in the right type.
    uint8_t *Ptr = (uint8_t*)PushSize(Vector->mem_arena, Vector->unit_size);
    
    Vector->total_size += Vector->unit_size;
    Vector->count++;
    return (void*)Ptr;
}

#define PopAndPeekVectorElement(Type,Vector) (Type*)PopAndPeekVectorElement_(Vector)
static void* PopAndPeekVectorElement_(YoyoVector* Vector)
{
    //TIMED_BLOCK();
    Assert(Vector);
    void* Result = GetVectorElement_(Vector,Vector->count);
    Vector->mem_arena->Used -= Vector->unit_size;
    Vector->total_size -= Vector->unit_size;
    Vector->count--;
    return Result;
}

static void PopVectorElement(YoyoVector* Vector)
{
    //TIMED_BLOCK();
    Assert(Vector);
    Vector->mem_arena->Used -= Vector->unit_size;
    Vector->total_size -= Vector->unit_size;
    Vector->count--;
}

#define IterateVector(Vector,Type) (Type*)IterateVectorElement_(Vector)
#define IterateVectorFromIndex(Vector,Type,Index) (Type*)IterateVectorElement_(Vector,Index)
#define IterateVectorFromToIndex(Vector,Type,Index,ToIndex) (Type*)IterateVectorElement_(Vector,Index,ToIndex)
static void* IterateVectorElement_(YoyoVector *Vector, s32 StartAt = -1,s32 EndAt = -1)
{
    //TIMED_BLOCK();
    Assert(Vector);
    if (Vector->at_index >= Vector->count)return 0;
    if (StartAt >= 0 && Vector->start_at < 0)
    {
        Vector->at_index = StartAt;
        Vector->start_at = StartAt;
    }
    else if(EndAt >= 0 && Vector->at_index == (u32)EndAt)
    {
        return 0;
    }
    return GetVectorElement_(Vector, Vector->at_index++);
}

static void ResetVectorIterator(YoyoVector *Vector)
{
    //TIMED_BLOCK();
    Assert(Vector);
    Vector->at_index = 0;
    Vector->start_at = -1;
}

static void ClearVector(YoyoVector *Vector)
{
    //TIMED_BLOCK();
    Assert(Vector);
    Vector->mem_arena->Used = 0;
    Vector->count = 0;
    Vector->at_index = 0;
    Vector->start_at = -1;
    Vector->mem_arena->TempCount = 0;
}

static void FreeVectorMem(YoyoVector *Vector)
{
    //TIMED_BLOCK();
    if(Vector->total_size > 0)
    {
        ClearVector(Vector);
		Vector->total_size = 0;
		Vector->total_count = 0;
        PlatformDeAllocateMemory(Vector->mem_arena->Base,Vector->mem_arena->Size);
		Vector->base = nullptr;
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
