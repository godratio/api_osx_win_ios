
//TODO(Ray):Returning pointers here is pretty stupid me thinks deprecate these oneday.
//Wrote this when I was a noob guy.

//NOTE(Ray):Now this api will guarentee that any Yostr type returned is null terminated if you want non null terminated strings
//figure it out later. Not a big deal to do that.

#if !defined(API_STRINGS_H)
/*
author: Ray Garner 
email : raygarner13@gmail.com

api_strings  - public domain string handling - 
                                     no warranty implied; use at your own risk
                                     
                                     LICENSE
  See end of file for license information.
  
  */



#define MAX_FILENAME_LENGTH 50
#define MAX_FILE_EXTENSION_LENGTH 3

#include <stdint.h>
#include <stdio.h>
#include "api_memory.h"

struct Yostr
{
    u32 NullTerminated;
    u32 Length;
    char* String;
};

struct fixed_element
{
    bool IsSentinal;
    Yostr** Data;
    fixed_element* Next;
};

struct strings
{
    Yostr* Strings;
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

api__inline b32 IsDigit(char Char);

//NOTE(ray):Assumes string is already null terminated.
APIDEF u32 String_GetLength_String(Yostr* String);

APIDEF u32 String_GetLengthSafely_String(Yostr* String,u32 SafetyLength);

//NOTE(ray):Assumes string is already null terminated.
APIDEF u32 String_GetLength_Char(char* String);

APIDEF u32 String_GetLengthSafely_Char(char* String,u32 SafetyLength);

//APIDEF Yostr NullTerminate(Yostr Source);

//APIDEF Yostr* CreateStringFromLiteralConst(const char* String,MemoryArena* Memory);

//TODO(ray):Make a way to reclaim the memory from literals created here.
APIDEF Yostr CreateStringFromLiteral(char* String,MemoryArena* Memory);

//TODO(Ray):Make a way to reclaim the memory from literals created here.
//TODO(Ray):Allow to have the option to do the length check safely.
//NOTE(Ray):This function requires you free your own memory once your done.
APIDEF Yostr* String_Allocate(char* String);

APIDEF Yostr AllocateEmptyString(MemoryArena* Partition);

APIDEF Yostr* CreateStringFromToChar(char* String,char* End, MemoryArena* Memory);

APIDEF Yostr* API_CreateStringFromToPointer_WithSplitMem(char* String, char* End,duel_memory_partition* Memory);

APIDEF Yostr* CreateStringFromToPointer(char* String, char* End, MemoryArena* Memory);

APIDEF Yostr* CreateStringFromLength(char* String,u32 Length,MemoryArena* Memory);

APIDEF int Compare(Yostr A, Yostr B);

APIDEF int CompareStringtoChar(Yostr A, char* B);
 
APIDEF int CompareCharToChar(char* A, char* B,u32 MaxIterations);

APIDEF b32 CompareChars(char *A, char *B);

//TODO(ray): Make sure this is never used in game.
APIDEF void PrintStringToConsole(Yostr String);

APIDEF Yostr* GetExtension(Yostr* FileNameOrPathWithExtension,MemoryArena *StringMem,b32 KeepFileExtensionDelimiter = false);

APIDEF Yostr* StripExtension(Yostr* FileNameOrPathWithExtension,MemoryArena *StringMem);

APIDEF Yostr* StripAndOutputExtension(Yostr* FileNameOrPathWithExtension,Yostr* Extension,MemoryArena *StringMem,b32 KeepFileExtensionDelimeter = false);

APIDEF Yostr* String_PadRight(Yostr* String,char PadChar,u32 PadAmount,MemoryArena* Memory);

APIDEF Yostr* EnforceMinSize(Yostr* String,u32 MinSize,MemoryArena* Memory);

#define AppendStringToChar(Front,Back,Memory) AppendString(CreateStringFromLiteral(Front,Memory),Back,Memory)
#define AppendCharToString(Front,Back,Memory) AppendString(Front,CreateStringFromLiteral(Back,Memory),Memory)

#define GetFilenameFromPath(yostr,arena)  GetFilenameFromPathChar(yostr.String,yostr.Length,arena)
APIDEF Yostr GetFilenameFromPathChar(char* pathwithfilename,uint64_t length,MemoryArena* string_mem);

APIDEF u32 CalculateStringLength(Yostr* String);

APIDEF u32 CalculateCharLength(char* String);

APIDEF Yostr* AppendString(Yostr Front,Yostr Back,MemoryArena* Memory);

#define AppendCharToStringAndAdvace(Front,Back,Memory) AppendStringAndAdvance(Front,CreateStringFromLiteral(Back,Memory),Memory)
APIDEF void AppendStringAndAdvance(Yostr* Front,Yostr Back,MemoryArena* Memory);

APIDEF Yostr* ElementIterator(fixed_element_size_list *Array);

#include "api_tokenizer.h"

APIDEF Yostr* GetFromStringsByIndex(strings Strings, u32 Index);

APIDEF strings API_String_Split(Yostr Source,char* Separator,duel_memory_partition* Memory);

APIDEF Yostr* API_String_Iterator(strings* StringArray);

//TODO(ray): Old function can do this much better.  REDO THIS!
//TODO(ray): This will fail in the case there is no seperator present in the string.
//Note(ray): The data type fixed_element... does not make sense should rename rework.
APIDEF fixed_element_size_list SplitString(Yostr Source,char* Separator,MemoryArena *Partition,bool SeparatorIsNotLastChar = false);
#define MAX_FORMAT_STRING_SIZE 500
APIDEF Yostr FormatToString(char* StringBuffer,MemoryArena* StringMemory);
#include <stdarg.h>
#include <stdio.h>
//TODO(ray): Move this to a more proper place replace std::out
APIDEF void PlatformOutputToConsole(b32 UseToggle,const char* FormatString,va_list list);
APIDEF void PlatformOutputInputPrompt(char* Buffer,b32 UseToggle,const char* FormatString,u32 _Dummy,...);
void PlatformOutput(bool use_toggle,const char* FormatString,...);

Yostr PlatformFormatString(MemoryArena *arena,char* format_string,...);

#ifdef YOYOIMPL

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
APIDEF u32 String_GetLength_String(Yostr* String)
{
    u32 Length = 0;
    char* At = String->String;
    while(*At)
    {
        Length++;
        At++;
    }
    String->Length = Length;
    return Length;
}

APIDEF u32 String_GetLengthSafely_String(Yostr* String,u32 SafetyLength)
{
    u32 Length = 0;
    char* At = String->String;
    while(*At)
    {
        Length++;
        At++;
        if(Length > SafetyLength)
        {
            break;
        }
    }
    String->Length = Length;
    return Length;
}

//NOTE(ray):Assumes string is already null terminated.
APIDEF u32 String_GetLength_Char(char* String)
{
    u32 Length = 0;
    char* At = String;
    while(*At)
    {
        Length++;
        At++;
    }
    return Length;
}

APIDEF u32 String_GetLengthSafely_Char(char* String,u32 SafetyLength)
{
    u32 Length = 0;
    char* At = String;
    while(*At)
    {
        Length++;
        At++;
        if(Length > SafetyLength)
        {
            break;
        }
    }
    return Length;
}

APIDEF Yostr NullTerminate(Yostr str,MemoryArena* mem)
{
    char *term_ptr = (char*)PushSize(mem,1);
    *term_ptr = '\0';
    str.NullTerminated = true;
    /*
    char* NullTerminatePoint = Source.String + Source.Length;
    *NullTerminatePoint = '\0';
    Source.NullTerminated = true;
    */
    return str;
}

/*
APIDEF Yostr CreateStringFromLiteralConst(const char* String,MemoryArena* Memory)
{
    Yostr Result = {};//(Yostr*)PushSize(Memory,sizeof(Yostr));
    char* At = (char*)String;
    void* StartPointer = GetPartitionPointer(*Memory);
	while (*At)
    {
        char * StringPtr = (char*)PushSize(Memory,1);
        *StringPtr = *At;
        Result.Length++;
        At++;
    }
    Result.String = (char*)StartPointer;
    return Result;
}
*/

APIDEF Yostr CreateStringFromLiteral(char* String,MemoryArena* Memory)
{
    Yostr Result = {};
    char* At = String;
    void* StartPointer = GetPartitionPointer(*Memory);
	while (*At)
    {
        char * StringPtr = (char*)PushSize(Memory,1);
        *StringPtr = *At;
        Result.Length++;
        At++;
    }
    Result.String = (char*)StartPointer;

    return NullTerminate(Result,Memory);
}

//TODO(Ray):Make a way to reclaim the memory from literals created here.
//TODO(Ray):Allow to have the option to do the length check safely.
//NOTE(Ray):This function requires you free your own memory once your done.
APIDEF Yostr* String_Allocate(char* String)
{
    u32 Length = String_GetLength_Char(String);
    void* Mem = PlatformAllocateMemory(sizeof(Yostr) + Length);
    Yostr* Result = (Yostr*)Mem;
    Result->Length = 0;
    char* At = String;
    char* StartPointer = (char*)Mem + sizeof(Yostr);
    char* StringPtr = (char*)Mem + sizeof(Yostr);
    while (*At)
    {
        *StringPtr = *At;
        StringPtr++;
//        Result->Length++;
        At++;
    }
    Result->String = StartPointer;
    Result->Length = Length;
    return Result;
}

APIDEF Yostr AllocateEmptyString(MemoryArena* Partition)
{
    Assert(Partition);
    return CreateStringFromLiteral("",Partition);
}

APIDEF Yostr* CreateStringFromToChar(char* String,char* End, MemoryArena* Memory)
{
    Yostr* Result = (Yostr*)PushSize(Memory, sizeof(Yostr));
    
    char* At = String;
    void* StartPointer = GetPartitionPointer(*Memory);
    char* StringPtr = 0;//(char*)Memory;
    while (*At != *End)
    {
        StringPtr = (char*)PushSize(Memory, 1);
        *StringPtr = *At;
        Result->Length++;
        At++;
    }
    Result->String = (char*)StartPointer;
    return Result;
}

APIDEF Yostr* API_CreateStringFromToPointer_WithSplitMem(char* String, char* End,duel_memory_partition* Memory)
{
    Yostr* Result = (Yostr*)PushSize(&Memory->FixedSized, sizeof(Yostr));
    
    char* At = String;
    void* StartPointer = GetPartitionPointer(Memory->VariableSized);
    char* StringPtr = 0;//(char*)Memory;
    while (At != End)
    {
        StringPtr = (char*)PushSize(&Memory->VariableSized, 1);
        *StringPtr = *At;
        Result->Length++;
        At++;
    }
    //One more for a possible null char.
    (char*)PushSize(&Memory->VariableSized, 1);
    Result->String = (char*)StartPointer;
    
    //*Result = NullTerminate(*Result);
    return Result;
}

APIDEF Yostr* CreateStringFromToPointer(char* String, char* End, MemoryArena* Memory)
{
    Yostr* Result = (Yostr*)PushSize(Memory, sizeof(Yostr));
    
    char* At = String;
    void* StartPointer = GetPartitionPointer(*Memory);
    char* StringPtr = 0;//(char*)Memory;
    while (At != End)
    {
        StringPtr = (char*)PushSize(Memory, 1);
        *StringPtr = *At;
        Result->Length++;
        At++;
    }
    Result->String = (char*)StartPointer;
    return Result;
}

APIDEF Yostr* CreateStringFromLength(char* String,u32 Length,MemoryArena* Memory)
{
    Yostr* Result = (Yostr*)PushSize(Memory,sizeof(Yostr));
    
    char* At = String;
    void* StartPointer = GetPartitionPointer(*Memory);
    char* StringPtr = 0;//(char*)Memory;
    u32 Iterator = 0;
    while (Iterator < Length)
    {
        StringPtr = (char*)PushSize(Memory,1);
        *StringPtr = *At;
        Result->Length++;
        At++;
        Iterator++;
    }
    Result->String = (char*)StartPointer;
    *Result = NullTerminate(*Result,Memory);
    return Result;
}

APIDEF int Compare(Yostr A, Yostr B)
{
    if (A.NullTerminated && B.NullTerminated)
    {
        char* APtr = A.String;
        char* BPtr = B.String;
        while (*APtr && *BPtr)
        {
            if (*APtr != *BPtr)
                return false;
            
            APtr++;BPtr++;
        }
    }
    else
    {
        char* APtr = A.String;
        char* BPtr = B.String;
        
        u32 MaxIterations = (A.Length > B.Length) ? A.Length : B.Length;
        for(u32 Index = 0;Index < MaxIterations;++Index)
        {
            if (*APtr != *BPtr)
                return false;
            
            APtr++; BPtr++;
        }
    }
    return true;
}

APIDEF int CompareStringtoChar(Yostr A, char* B)
{
    char* APtr = A.String;
    char* BPtr = B;
        
    u32 MaxIterations = A.Length;
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
APIDEF void PrintStringToConsole(Yostr String)
{
    //for(u32 CharIndex = 0;CharIndex < String.Length;++CharIndex)
    {
        //char* Char = (String.String + CharIndex);
        printf("%.*s", String.Length, String.String);
        //std::cout << *Char;
    }
}


APIDEF Yostr GetFilenameFromPathChar(char* pathwithfilename,uint64_t length,MemoryArena* string_mem)
{
    Yostr result;
    char* End = pathwithfilename + length;
    uint32_t StepCount = 1;
    while(*(End - 1) != '/' && StepCount <= (uint32_t)length)
    {
        --End;
        ++StepCount;
    }
    result.String = End;
    result.Length = StepCount;
    result = NullTerminate(result,string_mem);
    return result;
}

APIDEF Yostr* GetExtension(Yostr* FileNameOrPathWithExtension,MemoryArena *StringMem,b32 KeepFileExtensionDelimiter)
{
    Assert(FileNameOrPathWithExtension->Length > 1)
        //walk back from end of string till we hit a '.'
    char* End = FileNameOrPathWithExtension->String + FileNameOrPathWithExtension->Length - 1;
    u32 LookBack = 1;
    if(KeepFileExtensionDelimiter)
    {
        LookBack = 0;
    }
    u32 StepsTaken = 1;
    while (*(End - LookBack) != '.')
    {
        --End;
        ++StepsTaken;
        if (StepsTaken > MAX_FILE_EXTENSION_LENGTH)
        {
            Assert(false);//Something has failed horribly either the path was wrong or we do not support
            //extensions longer than 3 letters because its nonsensical
            
            break;
        }
    }
    Yostr* ExtensionName = CreateStringFromLength(End, StepsTaken, StringMem);
    return ExtensionName;
}

APIDEF Yostr* StripExtension(Yostr* FileNameOrPathWithExtension,MemoryArena *StringMem)
{
    Assert(FileNameOrPathWithExtension->Length > 1)
    
        //walk back from end of string till we hit a '.'
    char* End = FileNameOrPathWithExtension->String + FileNameOrPathWithExtension->Length - 1;
    u32 StepCount = 1;
    while (*End != '.')
    {
        --End;
        if (StepCount++ > FileNameOrPathWithExtension->Length)
        {
            //TODO(ray):Log this as an error?
            break;
        }
    }
    return CreateStringFromToChar(&FileNameOrPathWithExtension->String[0], &End[0], StringMem);
}

APIDEF Yostr* StripAndOutputExtension(Yostr* FileNameOrPathWithExtension,Yostr* Extension,MemoryArena *StringMem,b32 KeepFileExtensionDelimeter)
{
    Assert(FileNameOrPathWithExtension->Length > 1)
    
        Yostr* Result = StripExtension(FileNameOrPathWithExtension, StringMem);
    Yostr* ExtensionName = GetExtension(FileNameOrPathWithExtension, StringMem,KeepFileExtensionDelimeter);
    //string TerminatedExtensionName = NullTerminate(*ExtensionName);
    *Extension = *ExtensionName;
    return Result;
}

APIDEF Yostr* String_PadRight(Yostr* String,char PadChar,u32 PadAmount,MemoryArena* Memory)
{
	//TODO(RAY):LENGTH IS WRONG
    Yostr* Result = PushStruct(Memory,Yostr);
	Result->String = (char*)PushSize(Memory,String->Length + PadAmount);
    Result->Length = PadAmount + String->Length;
//    char* At = Result->String;
    char* SourceString = String->String;
//    while(*At++)
    for(u32 StringIndex = 0;StringIndex < Result->Length;++StringIndex)
    {
        char* At = Result->String + StringIndex;   
        if(StringIndex > String->Length - 1)
        {
            *At = PadChar;
            if(StringIndex <= (String->Length + PadAmount))
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

APIDEF Yostr* EnforceMinSize(Yostr* String,u32 MinSize,MemoryArena* Memory)
{
    if(String->Length < MinSize)
    {
        int Diff = MinSize - String->Length;
        String = String_PadRight(String,' ',Diff,Memory);
    }
    else if(String->Length > MinSize)
    {
        u32 Count = 0;
        char* At = String->String;

        while(*At++)
        {
            if(Count < MinSize + 2)
            {
                String->Length = MinSize;
                *At = '\0';
            }
            Count++;
        }
    }
    return String;
}

#define AppendStringToChar(Front,Back,Memory) AppendString(CreateStringFromLiteral(Front,Memory),Back,Memory)
#define AppendCharToString(Front,Back,Memory) AppendString(Front,CreateStringFromLiteral(Back,Memory),Memory)

APIDEF u32 CalculateStringLength(Yostr* String)
{
    u32 Length = 0;
    char* At = String->String;
    while(*At)
    {
        Length++;
        At++;
    }
    String->Length = Length;
    return Length;
}

APIDEF u32 CalculateCharLength(char* String)
{
    u32 Length = 0;
    char* At = String;
    while(*At)
    {
        Length++;
        At++;
    }
    return Length;
}

APIDEF Yostr* AppendString(Yostr Front,Yostr Back,MemoryArena* Memory)
{
    Yostr *Result = PushStruct(Memory,Yostr);
    void* StartPointer = GetPartitionPointer(*Memory);
    
    char* At = Front.String;
    u32 Iterations = 0;
    while(*At && Iterations < Front.Length)
    {
        char* StrPtr = (char*)PushSize(Memory,1);
        *StrPtr = *At;
        Result->Length++;
        At++;
        Iterations++;
    }
    At = Back.String;
    Iterations = 0;
    while(*At && Iterations < Back.Length)
    {
        char* StrPtr = (char*)PushSize(Memory,1);
        *StrPtr = *At;
        Result->Length++;
        At++;
        Iterations++;
    }
    Result->String = (char*)StartPointer;
    *Result = NullTerminate(*Result,Memory);
    return Result;
}

#define AppendCharToStringAndAdvace(Front,Back,Memory) AppendStringAndAdvance(Front,CreateStringFromLiteral(Back,Memory),Memory)
APIDEF void AppendStringAndAdvance(Yostr* Front,Yostr Back,MemoryArena* Memory)
{
    u32 Length = 0;
    void* StartPointer = GetPartitionPointer(*Memory);
    char* StrPtr;
    char* At = Front->String;
    u32 Iterations = 0;

    while(*At && Iterations < Front->Length)
    {
        StrPtr = (char*)PushSize(Memory,1);
        *StrPtr = *At;
        Length++;
        At++;
        Iterations++;
    }
    At = Back.String;
    Iterations = 0;

    while(*At && Iterations < Back.Length)
    {
        StrPtr = (char*)PushSize(Memory,1);
        *StrPtr = *At;
        Length++;
        At++;
        Iterations++;
    }

    Front->String = (char*)StartPointer;
    Front->Length = Length;
    *Front = NullTerminate(*Front,Memory);
}

APIDEF Yostr* ElementIterator(fixed_element_size_list *Array)
{
    Yostr* Result;
    
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

APIDEF Yostr* GetFromStringsByIndex(strings Strings, u32 Index)
{
    Assert(Strings.StringCount > Index)
        return (Strings.Strings + Index);
}

APIDEF strings API_String_Split(Yostr Source,char* Separator,duel_memory_partition* Memory)
{
    strings Result = {0};
//    Source = NullTerminate(Source,Memory);
    char* At = Source.String;
    char* Start  = At;
    b32 HasLastString = false;
	u32 CharIndex = 0;
    while(*At++ && Source.Length > CharIndex)
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
                Result.Strings = API_CreateStringFromToPointer_WithSplitMem(Start, At++, Memory);;
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
            
            Result.Strings = API_CreateStringFromToPointer_WithSplitMem(Start, At, Memory);
            Yostr * StringStart = Result.Strings;
        }
        else 
        {
            Yostr* P = API_CreateStringFromToPointer_WithSplitMem(Start, At, Memory);
        }
        Result.StringCount++;
    }
    return Result;
}

APIDEF Yostr* API_String_Iterator(strings* StringArray)
{
    Assert(StringArray->StringCount > 0)
        Assert(StringArray->Strings)
    
        if(StringArray->IteratorIndex > StringArray->StringCount - 1)
    {
        StringArray->IteratorIndex = 0;
        return 0;
    }
    else{
        Yostr* Result = StringArray->Strings + StringArray->IteratorIndex;
        StringArray->IteratorIndex++;
        return Result;
    }
}


//TODO(ray): Old function can do this much better.  REDO THIS!
//TODO(ray): This will fail in the case there is no seperator present in the string.
//Note(ray): The data type fixed_element... does not make sense should rename rework.
APIDEF fixed_element_size_list SplitString(Yostr Source,char* Separator,MemoryArena *Partition,bool SeparatorIsNotLastChar )
{
    
    fixed_element_size_list Result = {0};
    Result.UnitSize = sizeof(Yostr);
    
    char* At = Source.String;
    char* CurrentStart = Source.String;
    u32 Length = 0;
    u32 StringIndex = 0;
    while(*At && StringIndex < Source.Length)
    {
        if(*Separator == *At)
        {
            
            if(Result.Length == 0)
            {
                Yostr** Temp = (Yostr**)PushSize(Partition,sizeof(Yostr**));
                
                fixed_element* Element = PushStruct(Partition, fixed_element);
                fixed_element* SentinalElement = PushStruct(Partition, fixed_element);
                Element->IsSentinal = false;
                
                *Temp = CreateStringFromLength(CurrentStart,Length,Partition);
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
                Yostr** Temp = (Yostr**)PushSize(Partition,sizeof(Yostr**));
                fixed_element* Element = (fixed_element*)PushStruct(Partition,fixed_element);
                *Temp = CreateStringFromLength(CurrentStart,Length,Partition);
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
        Yostr** Temp = (Yostr**)PushSize(Partition,sizeof(Yostr**));
        fixed_element* Element = (fixed_element*)PushStruct(Partition,fixed_element);
        Element->IsSentinal = false;
        
        if(Result.Length == 0)
        {
            fixed_element* SentinalElement = (fixed_element*)PushStruct(Partition,fixed_element);
            Element->IsSentinal = false;
            
            *Temp = CreateStringFromLength(CurrentStart,Length,Partition);
            
            SentinalElement->IsSentinal = true;
            Result.Sentinal = SentinalElement;
            Element->Data = Temp;
            
            Result.Head = Result.Sentinal;
            Result.Head->Next = Element;
        }
        else
        {
            *Temp = CreateStringFromLength(CurrentStart,Length,Partition);
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
APIDEF Yostr FormatToString(char* StringBuffer,MemoryArena* StringMemory)
{
	char CharBuffer[MAX_FORMAT_STRING_SIZE];
#if OSX
    sprintf(CharBuffer, StringBuffer);
#elif IOS
    sprintf(CharBuffer, StringBuffer);
#elif WINDOWS
    sprintf_s(CharBuffer,StringBuffer);
#endif
    return CreateStringFromLiteral(CharBuffer,StringMemory);
}

#include <stdarg.h>
#include <stdio.h>
//TODO(ray): Move this to a more proper place replace std::out
APIDEF void PlatformOutputToConsole(b32 UseToggle,const char* FormatString,va_list list)
{
    if (UseToggle)
    {
        char TextBuffer[10000];
#if WINDOWS
		vsprintf(TextBuffer,FormatString, list);
		OutputDebugString(TextBuffer);
#elif OSX || IOS
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

void PlatformOutput(bool use_toggle,const char* FormatString,...)
{
    va_list List;
	va_start(List, FormatString);
    PlatformOutputToConsole(use_toggle,FormatString, List);
	va_end(List);
}

Yostr PlatformFormatString(MemoryArena *arena,char* format_string,...)
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
    Yostr result = CreateStringFromLiteral(TextBuffer,arena);
    va_end(list);
    return result;
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
 
#define API_STRINGS_H
#endif
