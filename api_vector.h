// DataStructures.cpp : Defines the entry point for the console application.
//
#if !defined(API_VECTOR)
#include <stdint.h>
#include <memory>

typedef uint32_t u32;
typedef uint32_t b32;

struct vector
{
	void* Base;
	u32 TotalSize;
	u32 UnitSize;
	u32 Count;
	u32 AtIndex;
	u32** FreeList;
	memory_partition* Partition;
};

static vector CreateVector(u32 StartSize,u32 UnitSize)
{
	Assert(StartSize > 0);
	Assert(UnitSize > 0)


	vector Result;
	
	Result.TotalSize = StartSize * UnitSize;
	Result.UnitSize = UnitSize;
	Result.Count = 0;
	Result.AtIndex = 0;
	//TODO(ray): change this to get memory froma a pre allocated partition.
	void* StartingMemory = malloc(Result.TotalSize);
	memory_partition* Partition = (memory_partition*)StartingMemory;
	AllocatePartition(Partition, Result.TotalSize,Partition+sizeof(memory_partition*));
	Result.Partition = Partition;
	Result.Base = Partition->Base;
	return Result;
}

#define GetVectorElement(Type,Vector,Index) (Type*)GetVectorElement_(Vector,Index)
#define GetVectorFirst(Type,Vector) (Type)GetVectorElement_(Vector,0) 
#define GetVectorLast(Type,Vector) (Type)GetVectorElement_(Vector,Vector->Count) 

static void* GetVectorElement_(vector* Vector, u32 Index)
{
	Assert(Vector);
	void* Location = (uint8_t*)Vector->Base + (Index * Vector->UnitSize);
	return Location;
}

static void PushVectorElement(vector* Vector, void* Element, b32 Copy = true)
{
	Assert(Vector && Element);
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

static void PopVectorElement(vector* Vector)
{
	Assert(Vector);
	Vector->Partition->Used -= Vector->UnitSize;
	Vector->TotalSize -= Vector->UnitSize;
	Vector->Count--;
}
/*//TODO(ray):Create a way to delete an arbiturary index and when inserting 
//use the indexes from the freelist before pushing on to the end.
static void DeleteVectorElement(vector* Vector,u32 Index)
{
	Assert(Vector);
	u32 Ptr = (u32)PushSize(Vector->Partition, sizeof(u32));
	**Ptr = Index;
	
}
*/
#define IterateVector(Vector,Type) (Type*)IterateVectorElement_(Vector)
#define IterateVectorFromIndex(Vector,Type,Index) (Type*)IterateVectorElement_(Vector,Index)
static void* IterateVectorElement_(vector *Vector, s32 StartAt = -1)
{
	Assert(Vector);
	if (Vector->AtIndex >= Vector->Count)return 0;
	if (StartAt < 0)StartAt = Vector->AtIndex;
	Vector->AtIndex = StartAt;
	return GetVectorElement_(Vector, Vector->AtIndex++);
}

static void ResetVectorIterator(vector *Vector)
{
	Assert(Vector);
	Vector->AtIndex = 0;
}


#define API_VECTOR
#endif
