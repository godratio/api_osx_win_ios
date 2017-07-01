// DataStructures.cpp : Defines the entry point for the console application.
//
#if !defined(API_VECTOR)
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
    memory_partition* Partition;
    u32 Pushable;
};
//TODO(ray):Add memory alignment options here
static vector CreateVector(u32 StartSize,u32 UnitSize)
{
    //TIMED_BLOCK();
    Assert(StartSize > 0);
    Assert(UnitSize > 0);
    
    vector Result;
    
    Result.TotalSize = StartSize * UnitSize;
    Result.UnitSize = UnitSize;
    Result.TotalCount = StartSize;
    Result.Count = 0;
    Result.AtIndex = 0;
    Result.StartAt = -1;
    Result.Pushable = true;
    //TODO(ray): change this to get memory froma a pre allocated partition.
    void* StartingMemory = PlatformAllocateMemory(Result.TotalSize);//VirtualAlloc(0, Result.TotalSize,
    //           MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE); //malloc(Result.TotalSize);
    memory_partition* Partition = (memory_partition*)StartingMemory;
    AllocatePartition(Partition, Result.TotalSize,Partition+sizeof(memory_partition*));
    Result.Partition = Partition;
    Result.Base = Partition->Base;
    return Result;
}

#define GetVectorElement(Type,Vector,Index) (Type*)GetVectorElement_(Vector,Index)
#define GetVectorFirst(Type,Vector) (Type)GetVectorElement_(Vector,0)
#define GetVectorLast(Type,Vector) (Type)GetVectorElement_(Vector,Vector.Count)
#define PeekVectorElement(Type,Vector) (Type*)GetVectorElement_(Vector,*Vector.Count-1)

static void* GetVectorElement_(vector* Vector, u32 Index)
{
    //TIMED_BLOCK();
    Assert(Vector);
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
static void PushVectorElement(vector* Vector, void* Element, b32 Copy = true)
{
    //TIMED_BLOCK();
    Assert(Vector && Element);
    Assert(Vector->Pushable);
    //TODO(ray):have some protection here to make sure we are added in the right type.
    uint8_t *Ptr = (uint8_t*)PushSize(Vector->Partition, Vector->UnitSize);
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
    Vector->Count++;
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

#define API_VECTOR
#endif
