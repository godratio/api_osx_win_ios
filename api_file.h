/*
author: Ray Garner
email : raygarner13@gmail.com

api_file  - public domain file handling - 
                                     no warranty implied; use at your own risk
                                     
                                     
                                     LICENSE
  See end of file for license information.
  
  */

#if !defined(API_FILE_H)
#include "api_strings.h"
#include "api_vector.h"


//#include <stdio.h>
//#include <iostream>
struct read_file_result
{
    s32 ContentSize;
    void* Content;
};

struct dir_files_result
{
    vector Files;
    
};

struct file_info
{
    void* File;
    string* Name;
    memory_index FileSize;
    u32 FileCount;
};

#if OSX
//Note(ray): User app needs to include core foundations need to do something about that.
static string* BuildPathToAssets(memory_partition *Partition)
{
    //string* CurrentDir = AllocatEmptyString(Partition);
    //string* DataPath = CreateStringFromLiteral("/../raze/data/",Partition);
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
    //TODO(ray):Verify this is big enough.
    u32 DirMaxSize = 1000;
    string* CurrentDir = AllocatEmptyString(Partition);
    PushSize(Partition,DirMaxSize);
    if (!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (u8 *)CurrentDir->String, DirMaxSize))
    {
        // error
    }
    CalculateStringLength(CurrentDir);
    return AppendString(*CurrentDir,*CreateStringFromLiteral("/", Partition),Partition);
}



static dir_files_result
OSXGetAllFilesInDir(string Path,memory_partition *StringMem)
{
    dir_files_result Result;
    Result.Files = CreateVector(1000,sizeof(file_info));
    
    char* WildCard = "\\*";
    string* WildCardPath = AppendString(Path, *CreateStringFromLiteral(WildCard,StringMem), StringMem);
    
    CFAllocatorRef alloc = CFAllocatorGetDefault();
    char* Dir = Path.String;
    CFStringRef DirRef = CFStringCreateWithCString(alloc, Path.String, kCFStringEncodingASCII);
    CFURLRef UrlRef = CFURLCreateWithString(alloc, DirRef, NULL);
    
    CFURLEnumeratorRef Enumerator = CFURLEnumeratorCreateForDirectoryURL(alloc, UrlRef, kCFURLEnumeratorDefaultBehavior, nullptr);
    CFURLRef URL = NULL;
    while (CFURLEnumeratorGetNextURL(Enumerator, &URL, NULL) == kCFURLEnumeratorSuccess) {
        CFNumberRef valueNum = NULL;
        
        CFMutableStringRef fileName =  CFStringCreateMutableCopy(kCFAllocatorDefault, 0, CFURLGetString(URL));
        
        const char *cs = CFStringGetCStringPtr( fileName, kCFStringEncodingMacRoman ) ;
        
        string* PathToFile = CreateStringFromLiteral((char*)cs, StringMem);
        
        char* End = PathToFile->String + PathToFile->Length - 1;
        u32 StepCount = 1;
        while(*(End - 1) != '/')
        {
            --End;
            ++StepCount;
        }
        
        string* FileName = CreateStringFromLength(End, StepCount, StringMem);
        file_info Info;
        
        Info.Name = FileName;// CreateStringFromLiteral(FileName,StringMem);// ffd.cFileName;
        
        PushVectorElement(&Result.Files, &Info);
        
        //puts( cs ) ; // works
        ///CFStringAppend(fileName, CFURLGetString(resUrl));
        //CFStringAppend(fileName, cfsName);
        
        if (CFURLCopyResourcePropertyForKey(URL, kCFURLFileSizeKey, &valueNum, nullptr) && (valueNum != NULL)) {
            //    signed long long int value;
            //    if (CFNumberGetValue(valueNum, kCFNumberLongLongType, &value) && (value >= 0))
            //        totalSize += value;
            
            //    if (self.verbose)
            //        printf("- %s\n", [[(__bridge NSURL *)URL path] UTF8String]);
            
            //    CFRelease(valueNum);
        }
    }
    
    //CFRelease(convertedRootURL);
    //NSString *docPath = @"/tmp";
    //SDirectoryEnumerator *dirEnum = [[NSFileManager defaultManager] enumeratorAtPath:docPath];
    
    //NSString *filename;
    /*
     while ((filename = [dirEnum nextObject])) {
     file_info Info;
     
     Info.Name = CreateStringFromLiteral(filename,StringMem);// ffd.cFileName;
     
     PushVectorElement(&Result.Files, &Info);
     //Do something with the file name
     }
     */
    return Result;
}

static read_file_result
OSXReadEntireFile(string Path)
{
    read_file_result Result;
    
    FILE *File = fopen (Path.String, "r");
    if (File == NULL)
    {
        Assert(File);
        printf("Ray Engine Error No file found");
    }
    else
    {
        
        u64 FileSize;
        fseek(File, 0, SEEK_END);
        FileSize = ftell(File);
        fseek(File, 0, SEEK_SET);
        
        kern_return_t kr;
        mach_vm_address_t address;
        Result.ContentSize = (s32)FileSize;
        
        mach_vm_size_t size = (mach_vm_size_t)FileSize;
        kr = mach_vm_allocate(mach_task_self(), &address, size, VM_FLAGS_ANYWHERE);
        
        Result.Content = (void*)address;//allocl mem;
        fread(Result.Content, Result.ContentSize, 1, File);
        fclose(File);
    }
    
    return Result;
}
#endif


#if WINDOWS
#include <windows.h>
#include "api_osx_win/api_memory.h"

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


static dir_files_result
Win32GetAllFilesInDir(string Path,memory_partition *StringMem)
{
    dir_files_result Result;
    
    char* WildCard = "\\*";
    string* WildCardPath = AppendString(Path, *CreateStringFromLiteral(WildCard,StringMem), StringMem);
    
    Result.Files = CreateVector(1000,sizeof(file_info));
    WIN32_FIND_DATA ffd;
    TCHAR szDir[MAX_PATH];
    HANDLE hFind = INVALID_HANDLE_VALUE;
    LARGE_INTEGER filesize;
    
    hFind = FindFirstFile(WildCardPath->String, &ffd);
    
    if (INVALID_HANDLE_VALUE == hFind)
    {
        return Result;
    }
    
    do
    {
        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            //_tprintf(TEXT("  %s   <DIR>\n"), ffd.cFileName);
        }
        else
        {
            filesize.LowPart = ffd.nFileSizeLow;
            filesize.HighPart = ffd.nFileSizeHigh;
            
            file_info Info;
            Info.Name = CreateStringFromLiteral(ffd.cFileName,StringMem);// ffd.cFileName;
            PushVectorElement(&Result.Files, &Info);
            
            //_tprintf(TEXT("  %s   %ld bytes\n"), ffd.cFileName, filesize.QuadPart);
        }
    } while (FindNextFile(hFind, &ffd) != 0);
    return Result;
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

#endif

static read_file_result PlatformReadEntireFile(string* FileName,memory_partition Memory)
{
#if WINDOWS
    string* AssetPath = BuildPathToAssets(&Memory);
    string* FinalPathToAsset = AppendString(*AssetPath,*CreateStringFromLiteral(FileName->String,&Memory),&Memory);
    NullTerminate(*FinalPathToAsset);
    return Win32ReadEntireFile(*FinalPathToAsset);
    
#elif OSX
    string* AssetPath = BuildPathToAssets(&Memory);
    string* FinalPathToAsset = AppendString(*AssetPath,*CreateStringFromLiteral(FileName->String,&Memory),&Memory);
    NullTerminate(*FinalPathToAsset);
    return OSXReadEntireFile(*FinalPathToAsset);
    
#endif
}


static dir_files_result PlatformGetAllFilesInDir(string TerminatedPathTestDir,memory_partition *StringMem)
{
#if WINDOWS
    return Win32GetAllFilesInDir(TerminatedPathTestDir, StringMem);
#elif OSX
    return OSXGetAllFilesInDir(TerminatedPathTestDir, StringMem);
#endif
}

#define API_FILE_H
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