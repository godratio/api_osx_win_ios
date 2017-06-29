#if !defined(API_WIN32_FILE)
#include <windows.h>
#include "api_win32_memory.h"
static string* BuildPathToAssets(memory_partition *Partition)
{
    //string* CurrentDir = AllocatEmptyString(Partition);
    string* DataPath = CreateStringFromLiteral("/../raze/data/",Partition);
    
    u32 MaxDirSize = Partition->Size;
    
    string* CurrentDir = AllocatEmptyString(Partition);
    u32 Size = GetCurrentDirectory(0,NULL);
    PushSize(Partition,Size);
    GetCurrentDirectory(Size,CurrentDir->String);
    CurrentDir->Length = Size;
    {
        //TODO(ray):Some error handling.
    }
    
    
    return AppendString(*CurrentDir,*DataPath,Partition);;
}

static read_file_result
Win32ReadEntireFile(string Path)
{
    //Assert(Path);
    read_file_result Result = {};
    
    HANDLE File;
    File = CreateFileA(
        Path.String,
        GENERIC_READ,
        FILE_SHARE_READ,
        0,
        OPEN_EXISTING,
        0,
        0
        );
    
    if(File != INVALID_HANDLE_VALUE)
    {
        
        LARGE_INTEGER FileSize;
        if(GetFileSizeEx(
            File,
            &FileSize
            ))
        {
            
            DWORD SizeResult;
            Result.Content = VirtualAlloc(
                0,
                FileSize.QuadPart,
                MEM_COMMIT | MEM_RESERVE,
                PAGE_READWRITE
                );
            
            if(Result.Content)
            {
                if(ReadFile(
                    File,
                    Result.Content,
                    FileSize.QuadPart,
                    &SizeResult,
                    0
                    ))
                {
                    Result.ContentSize = FileSize.QuadPart;
                    return Result;
                }
                else
                {
                    //TODO(ray):logging
                }
            }
            else
            {
                //TODO(ray):logging
                Win32DeAllocateMemory(Result.Content,Result.ContentSize);
            }
        }
        else
        {
            //TODO(ray):logging
        }
    }
    else
    {
        //TODO(ray):logging
        s32 ErrorCode = GetLastError();
        OutputDebugString("INVALIDHANDLE");
    }
    CloseHandle(File);
    return Result;
}



#define API_WIN32_FILE
#endif