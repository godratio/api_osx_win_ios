#include <stdint.h>
#if !defined(API_STRINGS)
typedef uint32_t u32;

struct memory_partition
{
	void* Base;
	u32 Size;
	u32 Used;
};

struct string
{
	u32 NullTerminated;
	u32 Length;
	char* String;
	char* LastChar;
};

struct fixed_element
{
	bool IsSentinal;
	string** Data;
	fixed_element* Next;
};

struct fixed_element_size_list
{
	fixed_element* Head;
	fixed_element* Sentinal;
	u32 Length;
	u32 UnitSize;

};

static void AllocatePartition(memory_partition *Partition, u32 Size, void* Base)
{
	//Assert
	Partition->Base = Base;
	Partition->Size = Size;
	Partition->Used = 0;
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

static void* GetPartitionPointer(memory_partition Partition)
{
	void* Result;
	Result = (uint8_t*)Partition.Base + Partition.Used;
	return Result;
}

#define PushStruct(Partition,Type) PushSize_(Partition,sizeof(Type))
#define PushSize(Partition,Size) PushSize_(Partition,Size)
#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
static void* PushSize_(memory_partition*Partition, u32 Size)
{
	//Assert Used < Size
	Assert(Partition->Used + Size < Partition->Size)
		void* Result;
	Result = (uint8_t*)Partition->Base + Partition->Used;
	Partition->Used = Partition->Used + Size;

	return Result;
}

static string* CreateStringFromLiteral(char* String, memory_partition* Memory)
{
	string* Result = (string*)PushSize(Memory, sizeof(string));

	char* At = String;
	void* StartPointer = GetPartitionPointer(*Memory);
	char* StringPtr = 0;//(char*)Memory;
	while (*At)
	{
		StringPtr = (char*)PushSize(Memory, 1);
		*StringPtr = *At;
		Result->Length++;
		At++;
	}
	Result->LastChar = StringPtr;
	Result->String = (char*)StartPointer;
	return Result;
}


static string* CreateStringFromLength(char* String, u32 Length, memory_partition* Memory)
{
	string* Result = (string*)PushSize(Memory, sizeof(string));

	char* At = String;
	void* StartPointer = GetPartitionPointer(*Memory);
	char* StringPtr = 0;//(char*)Memory;
	u32 Iterator = 0;
	while (Iterator < Length)
	{
		StringPtr = (char*)PushSize(Memory, 1);
		*StringPtr = *At;
		Result->Length++;
		At++;
		Iterator++;
	}
	Result->LastChar = StringPtr;
	Result->String = (char*)StartPointer;
	return Result;
}

static string NullTerminate(string Source)
{
	char* NullTerminatePoint = Source.String + Source.Length;
	*NullTerminatePoint = '\0';
	Source.NullTerminated = true;
	return Source;
}

static int Compare(string A, string B)
{
	if (A.NullTerminated && B.NullTerminated)
	{
		u32 Result;
		char* APtr = A.String;
		char* BPtr = B.String;
		while (*APtr && *BPtr)
		{
			if (*APtr != *BPtr)
				return false;

			APtr++; BPtr++;
		}
	}
	else
	{
		u32 Result;
		char* APtr = A.String;
		char* BPtr = B.String;

		u32 MaxIterations = (A.Length > B.Length) ? A.Length : B.Length;
		for (u32 Index = 0; Index < MaxIterations; ++Index)
		{
			if (*APtr != *BPtr)
				return false;

			APtr++; BPtr++;
		}
	}
	return true;
}

static void PrintStringToConsole(string String)
{
	for (u32 CharIndex = 0; CharIndex < String.Length; ++CharIndex)
	{
		char* Char = (String.String + CharIndex);
		std::cout << *Char;
	}
}

static string AppendString(string Front, string Back, memory_partition* Memory)
{
	string Result = { 0 };
	void* StartPointer = GetPartitionPointer(*Memory);
	char* StrPtr = 0;
	char* At = Front.String;
	u32 Iterations = 0;
	while (*At && Iterations < Front.Length)
	{
		StrPtr = (char*)PushSize(Memory, 1);
		*StrPtr = *At;
		Result.Length++;
		At++;
		Iterations++;
	}
	At = Back.String;
	Iterations = 0;
	while (*At && Iterations < Back.Length)
	{
		StrPtr = (char*)PushSize(Memory, 1);
		*StrPtr = *At;
		Result.Length++;
		At++;
		Iterations++;
	}
	Result.String = (char*)StartPointer;
	Result.LastChar = At;
	return Result;
}

static string* ElementIterator(fixed_element_size_list *Array)
{
	string* Result;

	if (Array->Head->IsSentinal)
	{
		Array->Head = Array->Head->Next;
		Result = *Array->Head->Data;

		return Result;
	}
	else
	{
		Array->Head = Array->Head->Next;
		if (!Array->Head)
		{
			return 0;
		}
		Result = *Array->Head->Data;

		return Result;
	}
	return 0;
}

static fixed_element_size_list SplitString(string Source, char* Separator, memory_partition *Partition, bool SeparatorIsLastChar = false)
{

	fixed_element_size_list Result = { 0 };
	Result.UnitSize = sizeof(string);

	char* At = Source.String;
	char* CurrentStart = Source.String;
	string** Head = 0;
	u32 Length = 0;
	u32 StringIndex = 0;
	while (*At && StringIndex < Source.Length)
	{
		if (*Separator == *At)
		{

			if (Result.Length == 0)
			{
				string** Temp = (string**)PushSize(Partition, sizeof(string**));

				fixed_element* Element = (fixed_element*)PushStruct(Partition, fixed_element);
				fixed_element* SentinalElement = (fixed_element*)PushStruct(Partition, fixed_element);
				Element->IsSentinal = false;

				*Temp = CreateStringFromLength(CurrentStart, Length, Partition);
				CurrentStart = At + 1;
				At++;//move past the separator
				StringIndex++;

				SentinalElement->IsSentinal = true;
				Result.Sentinal = SentinalElement;
				Element->Data = Temp;

				Result.Head = Result.Sentinal;
				Result.Head->Next = Element;
			}
			else
			{
				string** Temp = (string**)PushSize(Partition, sizeof(string**));
				fixed_element* Element = (fixed_element*)PushStruct(Partition, fixed_element);
				*Temp = CreateStringFromLength(CurrentStart, Length, Partition);
				Element->IsSentinal = false;

				CurrentStart = At + 1;
				At++;//Move past the separator
				StringIndex++;

				Element->Data = Temp;

				Result.Head = Result.Head->Next;
				Result.Head->Next = Element;

			}

			Result.Length++;
			Length = 0;
		}
		StringIndex++;
		At++;
		Length++;
	}
	if (SeparatorIsLastChar)
	{
		string** Temp = (string**)PushSize(Partition, sizeof(string**));
		fixed_element* Element = (fixed_element*)PushStruct(Partition, fixed_element);
		*Temp = CreateStringFromLength(CurrentStart, Length, Partition);
		Element->IsSentinal = false;

		Element->Data = Temp;

		Result.Head = Result.Head->Next;
		Result.Head->Next = Element;

		Result.Length++;
		Length = 0;
	}
	Result.Head = Result.Sentinal;
	return Result;
}

#define API_STRINGS
#endif
