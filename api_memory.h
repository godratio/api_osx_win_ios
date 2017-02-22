#include <stdint.h>

#if !defined(API_MEMORY)
typedef uint32_t u32;
typedef uint8_t u8;

typedef int32_t s32;

typedef uint32_t b32;
typedef float f32;
#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}


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

struct memory_partition
{
	void* Base;
	u32 Size;
	u32 Used;
	u32 TempCount;
};

struct temp_memory
{
	memory_partition *Partition;
	u32 Used;
};

static void AllocatePartition(memory_partition *Partition, u32 Size, void* Base)
{
	//Assert
	Partition->Base = Base;
	Partition->Size = Size;
	Partition->Used = 0;
	Partition->TempCount = 0;
}

//TODO(ray):Fix this to clear more effeciently. or give option for clearing  method
static void ClearToZero(memory_partition *Partition)
{
	for (u32 ByteIndex = 0; ByteIndex < Partition->Size; ++ByteIndex)
	{
		uint8_t* Byte = (uint8_t*)Partition->Base + ByteIndex;
		*Byte = 0;
	}
}

static void DeAllocatePartition(memory_partition *Partition, bool ClearMemToZero = true)
{
	//Assert
	Partition->Used = 0;
	if (ClearMemToZero)
	{
		ClearToZero(Partition);
	}
}

static temp_memory BeginTempMemory(memory_partition *Partition)
{
	temp_memory Result;
	Result.Partition = Partition;
	Partition->TempCount++;
	Result.Used = Partition->Used;
	return Result;
}

static void EndTempMemory(temp_memory TempMem)
{
	TempMem.Partition->Used = TempMem.Used;
	TempMem.Partition->TempCount--;
}

static void ValidateTempMemory(temp_memory TempMem)
{
	Assert(TempMem.Partition->TempCount < 1);
}

static b32 TestFlag(u32 Flag, u32 TestAgaist)
{
	if ((Flag & TestAgaist) == TestAgaist)
	{
		return true;
	}
	return false;
}

static void ClearSize(memory_partition *Partition,u32 Size)
{
	Assert(Size > 0)
	u8* Byte = (u8*)Partition->Base + Partition->Used;
	while (Size--)
	{
		*Byte++ = 0;
	}
}

#define PushStruct(Partition,Type,...) (Type*)PushSize_(Partition,sizeof(Type),## __VA_ARGS__)
#define PushSize(Partition,Size,...) PushSize_(Partition,Size,## __VA_ARGS__)
static void* PushSize_(memory_partition*Partition, u32 Size,partition_push_params PushParams = DefaultPartitionParams())
{
	//Assert Used < Size
	Assert(Partition->Used + Size < Partition->Size)
	void* Result;
	Result = (uint8_t*)Partition->Base + Partition->Used;
	if (TestFlag(PushParams.Flags, PartitionFlag_ClearToZero))
	{
		ClearSize(Partition, Size);
	}
	Partition->Used = Partition->Used + Size;

	return Result;
}

#define API_MEMORY
#endif

