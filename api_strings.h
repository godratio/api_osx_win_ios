/*
author: Ray Garner 
email : raygarner13@gmail.com

api_strings  - public domain string handling - 
                                     no warranty implied; use at your own risk
                                     
                                     LICENSE
  See end of file for license information.
  
  */

#if !defined(API_STRINGS_H)

#define MAX_FILENAME_LENGTH 50
#define MAX_FILE_EXTENSION_LENGTH 10

#include <stdint.h>
#include <stdio.h>
#include "api_memory.h"

struct YoyoAString
{
    bool null_terminated;
    size_t length;
    char* string;
};

api__inline b32 IsDigit(char Char)
{
    if ((Char == ' ' || Char == '.' ||
         Char > '9' || Char < '0'))
    {
        return false;
    }
    return true;
}

//NOTE(ray):Assumes string is already null terminated.
APIDEF size_t YoyoAsciiStringLength(YoyoAString* s)
{
    size_t length = 0;
    char* at = s->string;
    while(*at)
    {
        length++;
        at++;
    }
    s->length = length;
    return length;
}

APIDEF size_t YoyoAsciiStringLengthSafe(YoyoAString* s,size_t safe_length)
{
    size_t length = 0;
    char* at = s->string;
    while(*at)
    {
        length++;
        at++;
        if(length > safe_length)
        {
            break;
        }
    }
    s->length = length;
    return length;
}

//NOTE(ray):Assumes string is already null terminated.
APIDEF size_t YoyoAsciiGetLengthChar(char* s)
{
    size_t length = 0;
    char* at = s;
    while(*at)
    {
        length++;
        at++;
    }
    return length;
}

//TODO(Ray):Make a way to reclaim the memory from literals created here.
//TODO(Ray):Allow to have the option to do the length check safely.
//NOTE(Ray):This function requires you free your own memory once your done.
//Put here or convience not reccomended atm.
static YoyoAString* YoyoAsciiStringAllocate(char* s)
{
	uint32_t length = YoyoAsciiGetLengthChar(s);
	void* mem = malloc(sizeof(YoyoAString) + length);//PlatformAllocateMemory(sizeof(string) + Length);
	YoyoAString* result = (YoyoAString*)mem;
	result->length = 0;
	char* at = s;
	char* start_ptr = (char*)mem + sizeof(YoyoAString);
	char* string_ptr = (char*)mem + sizeof(YoyoAString);
	while (*at)
	{
		*string_ptr = *at;
		string_ptr++;
		at++;
	}
	result->string = start_ptr;
	result->length = length;
	return result;
}

APIDEF size_t YoyoAsciiGetLengthCharSafely(char* s,size_t safe_length)
{
    size_t length = 0;
    char* at = s;
    while(*at)
    {
        length++;
        at++;
        if(length > safe_length)
        {
            break;
        }
    }
    return length;
}

APIDEF YoyoAString YoyoAsciiNullTerminate(YoyoAString s)
{
    char* null_terminate_point = s.string + s.length;
    *null_terminate_point = '\0';
    s.null_terminated = true;
    return s;
}

//TODO(ray):Make a way to reclaim the memory
APIDEF YoyoAString* YoyoAsciiStringFromChar(char* s, MemoryArena* mem)
{
	YoyoAString* result = (YoyoAString*)PushSize(mem, sizeof(YoyoAString));
	result->length = 0;
	char* at = s;
	void* start_ptr = GetPartitionPointer(*mem);
	while (*at)
	{
		char * string_ptr = (char*)PushSize(mem, 1);
		*string_ptr = *at;
		result->length++;
		at++;
	}
	result->string = (char*)start_ptr;
	return result;
}

static YoyoAString* YoyoAsciiCreateStringRangedChar(char* s, char* e, MemoryArena* mem)
{
	YoyoAString* result = (YoyoAString*)PushSize(mem, sizeof(YoyoAString));
	char* at = s;
	void* start_ptr = GetPartitionPointer(*mem);
	char* string_ptr = 0;//(char*)Memory;
	while (*at != *e)
	{
		string_ptr = (char*)PushSize(mem, 1);
		*string_ptr = *at;
		result->length++;
		at++;
	}
	result->string = (char*)start_ptr;
	return result;
}

APIDEF YoyoAString* YoyoAsciiStringFromCharLength(char* s, u32 end_length, MemoryArena* mem)
{
	YoyoAString* result = (YoyoAString*)PushSize(mem, sizeof(YoyoAString));

	char* at = s;
	void* start_ptr = GetPartitionPointer(*mem);
	char* string_ptr = 0;//(char*)Memory;
	u32 iterator = 0;
	while (iterator < end_length)
	{
		string_ptr = (char*)PushSize(mem, 1);
		*string_ptr = *at;
		result->length++;
		at++;
		iterator++;
	}
	result->string = (char*)start_ptr;
	return result;
}

APIDEF YoyoAString* YoyoCreateStringRangedPointer(char* s, char* e, MemoryArena* mem)
{
	YoyoAString* result = (YoyoAString*)PushSize(mem, sizeof(YoyoAString));

	char* at = s;
	void* start_ptr = GetPartitionPointer(*mem);
	char* string_ptr = 0;//(char*)Memory;
	while (at != e)
	{
		string_ptr = (char*)PushSize(mem, 1);
		*string_ptr = *at;
		result->length++;
		at++;
	}
	result->string = (char*)start_ptr;
	return result;
}

static YoyoAString* YoyoAsciiAllocatEmptyString(MemoryArena* mem)
{
	Assert(mem);
	return YoyoAsciiStringFromChar("", mem);
}


APIDEF int YoyoAsciiStringCompare(YoyoAString a, YoyoAString b)
{
	if (a.null_terminated && b.null_terminated)
	{
		char* a_ptr = a.string;
		char* b_ptr = b.string;
		while (*a_ptr && *b_ptr)
		{
			if (*a_ptr != *b_ptr)
				return false;

			a_ptr++; b_ptr++;
		}
	}
	else
	{
		char* a_ptr = a.string;
		char* b_ptr = b.string;

		u32 max_iterations = (a.length > b.length) ? a.length : b.length;
		for (u32 index = 0; index < max_iterations; ++index)
		{
			if (*a_ptr != *b_ptr)
				return false;

			a_ptr++; b_ptr++;
		}
	}
	return true;
}

APIDEF int YoyoAsciiStringCompareToChar(YoyoAString a, char* b)
{
	char* a_ptr = a.string;
	char* b_ptr = b;

	u32 max_iterations = a.length;
	for (u32 index = 0; index < max_iterations; ++index)
	{
		//TODO(Ray):Need to check logic here for string where B is longer than A
		if (*a_ptr != *b_ptr)
			return false;
		a_ptr++; b_ptr++;
	}
	return true;
}

APIDEF int YoyoAsciiCharCompareToChar(char* a, char* b, u32 max_iterations)
{
	char* a_ptr = a;
	char* b_ptr = b;

	for (u32 index = 0; index < max_iterations; ++index)
	{
		if (*a_ptr == '\0' || *b_ptr == '\0')break;
		if (*a_ptr != *b_ptr)
			return false;
		a_ptr++; b_ptr++;
	}
	return true;
}


APIDEF YoyoAString* YoyoAsciiGetFileExtension(YoyoAString* file_name_or_path, MemoryArena *string_mem, b32 keep_extension_delimeter = false)
{
	Assert(file_name_or_path->length > 1)
		//walk back from end of string till we hit a '.'
		char* end = file_name_or_path->string + file_name_or_path->length - 1;
	u32 look_back = 1;
	if (keep_extension_delimeter)
	{
		look_back = 0;
	}
	u32 steps_taken = 1;
	while (*(end - look_back) != '.')
	{
		--end;
		++steps_taken;
		if (steps_taken > MAX_FILE_EXTENSION_LENGTH)
		{
			//TODO(ray):Log this as an error?
			break;
		}
	}
	YoyoAString* extension_name = YoyoAsciiStringFromCharLength(end, steps_taken, string_mem);
	return extension_name;
}

APIDEF YoyoAString* YoyoAsciiStripExtension(YoyoAString* file_name_or_path, MemoryArena *mem)
{
	Assert(file_name_or_path->length > 1);
	//walk back from end of string till we hit a '.'
	char* end = file_name_or_path->string + file_name_or_path->length - 1;
	u32 step_count = 1;
	while (*end != '.')
	{
		--end;
		if (step_count > MAX_FILENAME_LENGTH)
		{
			//TODO(ray):Log this as an error?
			break;
		}
	}
	return YoyoAsciiCreateStringRangedChar(&file_name_or_path->string[0], &end[0], mem);
}


APIDEF YoyoAString* YoyoAsciiStripAndOutputExtension(YoyoAString* file_name_or_path, YoyoAString* ext, MemoryArena *mem, b32 keep_delimeter = false)
{
	Assert(file_name_or_path->length > 1)

		YoyoAString* result = YoyoAsciiStripExtension(file_name_or_path, mem);
	YoyoAString* extension_name = YoyoAsciiGetFileExtension(file_name_or_path, mem, keep_delimeter);
	//string TerminatedExtensionName = NullTerminate(*ExtensionName);
	*ext = *extension_name;
	return result;
}

APIDEF YoyoAString* YoyoAsciiPadRight(YoyoAString* s, char pad_char, u32 pad_amount, MemoryArena* mem)
{
	//TODO(RAY):LENGTH IS WRONG
	YoyoAString* result = PushStruct(mem, YoyoAString);
	result->string = (char*)PushSize(mem, s->length + pad_amount);
	result->length = pad_amount + s->length;
	//    char* At = Result->String;
	char* source_string = s->string;
	//    while(*At++)
	for (u32 string_index = 0; string_index < result->length; ++string_index)
	{
		char* at = result->string + string_index;
		if (string_index > s->length - 1)
		{
			*at = pad_char;
			if (string_index <= (s->length + pad_amount))
			{
				at++;
				*at = '\0';
			}
		}
		else
		{
			*at = *source_string++;
		}
	}
	return result;
}

APIDEF YoyoAString* YoyoAsciiEnforceMinSize(YoyoAString* s, u32 min_size, MemoryArena* mem)
{
	if (s->length < min_size)
	{
		int Diff = min_size - s->length;
		s = YoyoAsciiPadRight(s, ' ', Diff, mem);
	}
	else if (s->length > min_size)
	{
		u32 count = 0;
		char* at = s->string;

		while (*at++)
		{
			if (count < min_size + 2)
			{
				s->length = min_size;
				*at = '\0';
			}
			count++;
		}
	}
	return s;
}

#define YoyoAsciiAppendStringToChar(front,back,mem) YoyoAsciiAppendString(*YoyoAsciiStringFromChar(front,mem),back,mem)
#define YoyoAsciiAppendCharToString(front,back,mem) YoyoAsciiAppendString(front,*YoyoAsciiStringFromChar(back,mem),mem)

APIDEF YoyoAString* YoyoAsciiAppendString(YoyoAString front, YoyoAString back, MemoryArena* mem)
{
	YoyoAString *result = PushStruct(mem, YoyoAString);
	void* start_ptr = GetPartitionPointer(*mem);
	char* string_ptr;
	char* at = front.string;
	u32 iterations = 0;
	while (*at && iterations < front.length)
	{
		string_ptr = (char*)PushSize(mem, 1);
		*string_ptr = *at;
		result->length++;
		at++;
		iterations++;
	}
	at = back.string;
	iterations = 0;
	while (*at && iterations < back.length)
	{
		string_ptr = (char*)PushSize(mem, 1);
		*string_ptr = *at;
		result->length++;
		at++;
		iterations++;
	}
	result->string = (char*)start_ptr;
	*result = YoyoAsciiNullTerminate(*result);
	return result;
}

#define YoyoAppendCharToStringAndAdvace(front,back,mem) YoyoAppendStringAndAdvance(front,*YoyoAsciiStringFromChar(back,mem),mem)
APIDEF void YoyoAppendStringAndAdvance(YoyoAString* front, YoyoAString back, MemoryArena* mem)
{
	u32 length = 0;
	void* start_pointer = GetPartitionPointer(*mem);
	char* str_ptr;
	char* at = front->string;
	u32 iterations = 0;

	while (*at && iterations < front->length)
	{
		str_ptr = (char*)PushSize(mem, 1);
		*str_ptr = *at;
		length++;
		at++;
		iterations++;
	}
	at = back.string;
	iterations = 0;

	while (*at && iterations < back.length)
	{
		str_ptr = (char*)PushSize(mem, 1);
		*str_ptr = *at;
		length++;
		at++;
		iterations++;
	}

	front->string = (char*)start_pointer;
	front->length = length;
	*front = YoyoAsciiNullTerminate(*front);
}



/////////////////////////////////////////////////////////////////////////////////////////
//TODO(ray):Make a way to reclaim the memory

#define AppendCharToStringAndAdvace(Front,Back,Memory) AppendStringAndAdvance(Front,*CreateStringFromLiteral(Back,Memory),Memory)
APIDEF void AppendStringAndAdvance(YoyoAString* Front, YoyoAString Back, MemoryArena* Memory)
{
	u32 Length = 0;
	void* StartPointer = GetPartitionPointer(*Memory);
	char* StrPtr;
	char* At = Front->string;
	u32 Iterations = 0;

	while (*At && Iterations < Front->length)
	{
		StrPtr = (char*)PushSize(Memory, 1);
		*StrPtr = *At;
		Length++;
		At++;
		Iterations++;
	}
	At = Back.string;
	Iterations = 0;

	while (*At && Iterations < Back.length)
	{
		StrPtr = (char*)PushSize(Memory, 1);
		*StrPtr = *At;
		Length++;
		At++;
		Iterations++;
	}

	Front->string = (char*)StartPointer;
	Front->length = Length;
	*Front = YoyoAsciiNullTerminate(*Front);
}

APIDEF YoyoAString* CreateStringFromLiteral(char* String,MemoryArena* Memory)
{
    YoyoAString* Result = (YoyoAString*)PushSize(Memory,sizeof(YoyoAString));
    Result->length = 0;
    char* At = String;
    void* StartPointer = GetPartitionPointer(*Memory);
	while (*At)
    {
        char * StringPtr = (char*)PushSize(Memory,1);
        *StringPtr = *At;
        Result->length++;
        At++;
    }
    Result->string = (char*)StartPointer;
    return Result;
}

//TODO(Ray):Make a way to reclaim the memory from literals created here.
//TODO(Ray):Allow to have the option to do the length check safely.
//NOTE(Ray):This function requires you free your own memory once your done.
APIDEF YoyoAString* String_Allocate(char* String)
{
    u32 Length = YoyoAsciiGetLengthChar(String);
    void* Mem = PlatformAllocateMemory(sizeof(YoyoAString) + Length);
    YoyoAString* Result = (YoyoAString*)Mem;
    Result->length = 0;
    char* At = String;
    char* StartPointer = (char*)Mem + sizeof(YoyoAString);
    char* StringPtr = (char*)Mem + sizeof(YoyoAString);
    while (*At)
    {
        *StringPtr = *At;
        StringPtr++;
//        Result->Length++;
        At++;
    }
    Result->string = StartPointer;
    Result->length = Length;
    return Result;
}

APIDEF YoyoAString* AllocatEmptyString(MemoryArena* Partition)
{
    Assert(Partition);
    return CreateStringFromLiteral("",Partition);
}

APIDEF YoyoAString* CreateStringFromToChar(char* String,char* End, MemoryArena* Memory)
{
    YoyoAString* Result = (YoyoAString*)PushSize(Memory, sizeof(YoyoAString));
    
    char* At = String;
    void* StartPointer = GetPartitionPointer(*Memory);
    char* StringPtr = 0;//(char*)Memory;
    while (*At != *End)
    {
        StringPtr = (char*)PushSize(Memory, 1);
        *StringPtr = *At;
        Result->length++;
        At++;
    }
    Result->string = (char*)StartPointer;
    return Result;
}


//TODO(Ray):REWRITE
APIDEF YoyoAString* API_CreateStringFromToPointer_WithSplitMem(char* String, char* End,duel_memory_partition* Memory)
{
    YoyoAString* Result = (YoyoAString*)PushSize(&Memory->FixedSized, sizeof(YoyoAString));
    
    char* At = String;
    void* StartPointer = GetPartitionPointer(Memory->VariableSized);
    char* StringPtr = 0;//(char*)Memory;
    while (At != End)
    {
        StringPtr = (char*)PushSize(&Memory->VariableSized, 1);
        *StringPtr = *At;
        Result->length++;
        At++;
    }
    //One more for a possible null char.
    (char*)PushSize(&Memory->VariableSized, 1);
    Result->string = (char*)StartPointer;
    //*Result = NullTerminate(*Result);
    return Result;
}

APIDEF int Compare(YoyoAString A, YoyoAString B)
{
    if (A.null_terminated && B.null_terminated)
    {
        char* APtr = A.string;
        char* BPtr = B.string;
        while (*APtr && *BPtr)
        {
            if (*APtr != *BPtr)
                return false;
            
            APtr++;BPtr++;
        }
    }
    else
    {
        char* APtr = A.string;
        char* BPtr = B.string;
        
        u32 MaxIterations = (A.length > B.length) ? A.length : B.length;
        for(u32 Index = 0;Index < MaxIterations;++Index)
        {
            if (*APtr != *BPtr)
                return false;
            
            APtr++; BPtr++;
        }
    }
    return true;
}

APIDEF int CompareStringtoChar(YoyoAString A, char* B)
{
    char* APtr = A.string;
    char* BPtr = B;
        
    u32 MaxIterations = A.length;
    for(u32 Index = 0;Index < MaxIterations;++Index)
    {
//TODO(Ray):Need to check logic here for string where B is longer than A
        if (*APtr != *BPtr)
            return false;
        APtr++; BPtr++;
    }
    return true;
}
 
APIDEF int CompareCharToChar(char* A, char* B,u32 MaxIterations)
{
    char* APtr = A;
    char* BPtr = B;
        
    for(u32 Index = 0;Index < MaxIterations;++Index)
    {
        if(*APtr == '\0' || *BPtr == '\0')break;
        if (*APtr != *BPtr)
            return false;
        APtr++; BPtr++;
    }
    return true;
}

APIDEF b32 CompareChars(char *A, char *B)
{
    b32 Result = (A == B);
    
    if(A && B)
    {
        while(*A && *B && (*A == *B))
        {
            ++A;
            ++B;
        }
        
        Result = ((*A == 0) && (*B == 0));
    }
    return(Result);
}

//TODO(ray): Make sure this is never used in game.
APIDEF void PrintStringToConsole(YoyoAString String)
{
    //for(u32 CharIndex = 0;CharIndex < String.Length;++CharIndex)
    {
        //char* Char = (String.String + CharIndex);
        printf("%.*s", String.length, String.string);
        //std::cout << *Char;
    }
}



APIDEF YoyoAString* StripExtension(YoyoAString* FileNameOrPathWithExtension,MemoryArena *StringMem)
{
    Assert(FileNameOrPathWithExtension->length > 1)
    
        //walk back from end of string till we hit a '.'
    char* End = FileNameOrPathWithExtension->string + FileNameOrPathWithExtension->length - 1;
    u32 StepCount = 1;
    while (*End != '.')
    {
        --End;
        if (StepCount > MAX_FILENAME_LENGTH)
        {
            //TODO(ray):Log this as an error?
            break;
        }
    }
    return CreateStringFromToChar(&FileNameOrPathWithExtension->string[0], &End[0], StringMem);
}

APIDEF YoyoAString* StripAndOutputExtension(YoyoAString* FileNameOrPathWithExtension,YoyoAString* Extension,MemoryArena *StringMem,b32 KeepFileExtensionDelimeter = false)
{
    Assert(FileNameOrPathWithExtension->length > 1)
    
        YoyoAString* Result = StripExtension(FileNameOrPathWithExtension, StringMem);
    YoyoAString* ExtensionName = YoyoAsciiGetFileExtension(FileNameOrPathWithExtension, StringMem,KeepFileExtensionDelimeter);
    //string TerminatedExtensionName = NullTerminate(*ExtensionName);
    *Extension = *ExtensionName;
    return Result;
}

APIDEF YoyoAString* String_PadRight(YoyoAString* String,char PadChar,u32 PadAmount,MemoryArena* Memory)
{
	//TODO(RAY):LENGTH IS WRONG
    YoyoAString* Result = PushStruct(Memory,YoyoAString);
	Result->string = (char*)PushSize(Memory,String->length + PadAmount);
    Result->length = PadAmount + String->length;
//    char* At = Result->String;
    char* SourceString = String->string;
//    while(*At++)
    for(u32 StringIndex = 0;StringIndex < Result->length;++StringIndex)
    {
        char* At = Result->string + StringIndex;   
        if(StringIndex > String->length - 1)
        {
            *At = PadChar;
            if(StringIndex <= (String->length + PadAmount))
            {
                At++;
                *At = '\0';
            }
        }
        else
        {
            *At = *SourceString++;
        }
    }
    return Result;
}

APIDEF YoyoAString* EnforceMinSize(YoyoAString* String,u32 MinSize,MemoryArena* Memory)
{
    if(String->length < MinSize)
    {
        int Diff = MinSize - String->length;
        String = String_PadRight(String,' ',Diff,Memory);
    }
    else if(String->length > MinSize)
    {
        u32 Count = 0;
        char* At = String->string;

        while(*At++)
        {
            if(Count < MinSize + 2)
            {
                String->length = MinSize;
                *At = '\0';
            }
            Count++;
        }
    }
    return String;
}

#define AppendStringToChar(Front,Back,Memory) AppendString(*CreateStringFromLiteral(Front,Memory),Back,Memory)
#define AppendCharToString(Front,Back,Memory) AppendString(Front,*CreateStringFromLiteral(Back,Memory),Memory)


APIDEF YoyoAString* AppendString(YoyoAString Front,YoyoAString Back,MemoryArena* Memory)
{
    YoyoAString *Result = PushStruct(Memory,YoyoAString);
    void* StartPointer = GetPartitionPointer(*Memory);
    char* StrPtr;
    char* At = Front.string;
    u32 Iterations = 0;
    while(*At && Iterations < Front.length)
    {
        StrPtr = (char*)PushSize(Memory,1);
        *StrPtr = *At;
        Result->length++;
        At++;
        Iterations++;
    }
    At = Back.string;
    Iterations = 0;
    while(*At && Iterations < Back.length)
    {
        StrPtr = (char*)PushSize(Memory,1);
        *StrPtr = *At;
        Result->length++;
        At++;
        Iterations++;
    }
    Result->string = (char*)StartPointer;
    *Result = YoyoAsciiNullTerminate(*Result);
    return Result;
}


//
struct fixed_element
{
	bool IsSentinal;
	YoyoAString** Data;
	fixed_element* Next;
};

struct temp_string
{
	YoyoAString* temp_strings;
	u32 StringCount;
	u32 IteratorIndex;
};

struct fixed_element_size_list
{
	fixed_element* Head;
	fixed_element* Sentinal;
	u32 Length;
	u32 UnitSize;

};

APIDEF u32 CalculateStringLength(YoyoAString* String)
{
	u32 Length = 0;
	char* At = String->string;
	while (*At)
	{
		Length++;
		At++;
	}
	String->length = Length;
	return Length;
}

APIDEF u32 CalculateCharLength(char* String)
{
	u32 Length = 0;
	char* At = String;
	while (*At)
	{
		Length++;
		At++;
	}
	return Length;
}

APIDEF YoyoAString* ElementIterator(fixed_element_size_list *Array)
{
    YoyoAString* Result;
    
    if(Array->Head->IsSentinal)
    {
        Array->Head = Array->Head->Next;
        Result = *Array->Head->Data;
        
        return Result;
    }
    else
    {
        Array->Head = Array->Head->Next;
        if(!Array->Head)
        {
            return 0;
        }
        Result = *Array->Head->Data;
        
        return Result;
    }
    //return 0;
}
#include "api_tokenizer.h"

APIDEF YoyoAString* GetFromStringsByIndex(temp_string Strings, u32 Index)
{
    Assert(Strings.StringCount > Index)
        return (Strings.temp_strings + Index);
}

APIDEF temp_string API_String_Split(YoyoAString Source,char* Separator,duel_memory_partition* Memory)
{
    temp_string Result = {0};
    Source = YoyoAsciiNullTerminate(Source);
    char* At = Source.string;
    char* Start  = At;
    b32 HasLastString = false;
	u32 CharIndex = 0;
    while(*At++ && Source.length > CharIndex)
    {
        HasLastString = true;
        if(*At == *Separator)
        {
            while(IsWhiteSpace(*Start))
            {
                Start++;
            }
            u32 MovedBackCount = 0;
            while(IsWhiteSpace(*(At - 1)))
            {
                MovedBackCount++;
                At--;
            }
            if(Result.StringCount == 0)
            {
                Result.temp_strings = API_CreateStringFromToPointer_WithSplitMem(Start, At++, Memory);;
            }
            else
            {
                //TODO(Ray):After looking throught his i seen i left there here i think we can
                //safe remove this.
                //string* R = API_CreateStringFromToPointer_WithSplitMem(Start, At++, Memory);
            }
            Result.StringCount++;
            At = At + MovedBackCount;
            Start = At;
            HasLastString = false;
        }
        CharIndex++;
    }
    if (HasLastString)
    {
        while(IsWhiteSpace(*Start))
        {
            Start++;
        }
        u32 MovedBackCount = 0;
        while(IsWhiteSpace(*(At - 1)))
        {
            MovedBackCount++;
            At--;
        }
        if (Result.StringCount == 0)
        {
            
            Result.temp_strings = API_CreateStringFromToPointer_WithSplitMem(Start, At, Memory);
            YoyoAString * StringStart = Result.temp_strings;
        }
        else 
        {
            YoyoAString* P = API_CreateStringFromToPointer_WithSplitMem(Start, At, Memory);
        }
        Result.StringCount++;
    }
    return Result;
}

APIDEF YoyoAString* API_String_Iterator(temp_string* StringArray)
{
    Assert(StringArray->StringCount > 0)
        Assert(StringArray->temp_strings)
    
        if(StringArray->IteratorIndex > StringArray->StringCount - 1)
    {
        StringArray->IteratorIndex = 0;
        return 0;
    }
    else{
        YoyoAString* Result = StringArray->temp_strings + StringArray->IteratorIndex;
        StringArray->IteratorIndex++;
        return Result;
    }
}


//TODO(ray): Old function can do this much better.  REDO THIS!
//TODO(ray): This will fail in the case there is no seperator present in the string.
//Note(ray): The data type fixed_element... does not make sense should rename rework.
APIDEF fixed_element_size_list SplitString(YoyoAString Source,char* Separator,MemoryArena *Partition,bool SeparatorIsNotLastChar = false)
{
    
    fixed_element_size_list Result = {0};
    Result.UnitSize = sizeof(YoyoAString);
    
    char* At = Source.string;
    char* CurrentStart = Source.string;
    u32 Length = 0;
    u32 StringIndex = 0;
    while(*At && StringIndex < Source.length)
    {
        if(*Separator == *At)
        {
            
            if(Result.Length == 0)
            {
                YoyoAString** Temp = (YoyoAString**)PushSize(Partition,sizeof(YoyoAString**));
                
                fixed_element* Element = PushStruct(Partition, fixed_element);
                fixed_element* SentinalElement = PushStruct(Partition, fixed_element);
                Element->IsSentinal = false;
                
                *Temp = YoyoAsciiStringFromCharLength(CurrentStart,Length,Partition);
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
                YoyoAString** Temp = (YoyoAString**)PushSize(Partition,sizeof(YoyoAString**));
                fixed_element* Element = (fixed_element*)PushStruct(Partition,fixed_element);
                *Temp = YoyoAsciiStringFromCharLength(CurrentStart,Length,Partition);
                Element->IsSentinal = false;
                
                CurrentStart = At+1;
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
    if(SeparatorIsNotLastChar)
    {
        YoyoAString** Temp = (YoyoAString**)PushSize(Partition,sizeof(YoyoAString**));
        fixed_element* Element = (fixed_element*)PushStruct(Partition,fixed_element);
        Element->IsSentinal = false;
        
        if(Result.Length == 0)
        {
            fixed_element* SentinalElement = (fixed_element*)PushStruct(Partition,fixed_element);
            Element->IsSentinal = false;
            
            *Temp = YoyoAsciiStringFromCharLength(CurrentStart,Length,Partition);
            
            SentinalElement->IsSentinal = true;
            Result.Sentinal = SentinalElement;
            Element->Data = Temp;
            
            Result.Head = Result.Sentinal;
            Result.Head->Next = Element;
        }
        else
        {
            *Temp = YoyoAsciiStringFromCharLength(CurrentStart,Length,Partition);
            Element->Data = Temp;
            
            Result.Head = Result.Head->Next;
            Result.Head->Next = Element;
        }
        Result.Length++;
    }
    Result.Head = Result.Sentinal;
    return Result;
}
 
//TODO(RAY):THIS IS GARBAGE USELESS what was I thinking.
//need to have most of these functions easy to format strings.
#define MAX_FORMAT_STRING_SIZE 500
APIDEF YoyoAString* FormatToString(char* StringBuffer,MemoryArena* StringMemory)
{
	char CharBuffer[MAX_FORMAT_STRING_SIZE];
#if OSX
    sprintf(CharBuffer, StringBuffer);
#elif IOS
    sprintf(CharBuffer, StringBuffer);
#elif WINDOWS
    sprintf_s(CharBuffer,StringBuffer);
#endif
    YoyoAString * Result = CreateStringFromLiteral(CharBuffer,StringMemory);
    return Result;
}

#include <stdarg.h>
#include <stdio.h>
//TODO(ray): Move this to a more proper place replace std::out
APIDEF void PlatformOutputToConsole(b32 UseToggle,const char* FormatString,va_list list)
{
    if (UseToggle)
    {
        char TextBuffer[100];
#if WINDOWS
		vsprintf(TextBuffer,FormatString, list);
		printf("%s",TextBuffer);
#elif OSX
        vsprintf(TextBuffer, FormatString, list);
        printf("%s",TextBuffer);
#endif
    }
}

APIDEF void WaitForInput()
{
	char buffer[2048];
	fgets(buffer, 2048, stdin);
}

APIDEF void PlatformOutputInputPrompt(char* Buffer,b32 UseToggle,const char* FormatString,u32 _Dummy,...)
{
    
    PlatformOutputToConsole(UseToggle,FormatString,0);
    fgets(Buffer,2048,stdin);
}

APIDEF void PlatformOutput(bool use_toggle,const char* FormatString,...)
{
    va_list List;
	va_start(List, FormatString);
	char TextBuffer[100];
    PlatformOutputToConsole(use_toggle,FormatString, List);
	va_end(List);
}

static YoyoAString* PlatformFormatString(MemoryArena *arena,char* format_string,...)
{
    
    va_list list;
    va_start(list,format_string);
    char TextBuffer[1000];
#if OSX
    vsprintf(TextBuffer,format_string,list);
#elif WINDOWS
    vsprintf(TextBuffer,format_string,list);
#elif IOS
    vsprintf(TextBuffer,format_string,list);
#endif
    YoyoAString* result = CreateStringFromLiteral(TextBuffer,arena);
    va_end(list);
    return result;
}

#define API_STRINGS_H
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
