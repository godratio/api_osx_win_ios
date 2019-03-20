
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
#include "api_memory.h"

//NOTE(Ray):We will no longer dicate this.
//IMPORTANT(RAY):THIS MUST BE SET.
namespace APIFileOptions
{
    extern char* data_dir;
}

static char* DataDir = "/../../data/";

//#include <stdio.h>
//#include <iostream>
struct read_file_result
{
	s32 ContentSize;
	void* Content;
};

struct dir_files_result
{
	vector Files;//files infos
	~dir_files_result()
	{
		//FreeVectorMem(&Files);
	}
};

struct file_info
{
	void* File;
	Yostr Name;
	memory_index FileSize;
	u32 FileCount;
};

enum directory_type
{
    Directory_None,
    Directory_Models,
    Directory_Materials,
    Directory_Shaders,
    Directory_Textures,
    Directory_Sounds,
    Directory_Fonts,
    Directory_Lighting
};

struct PlatformFilePointer
{
#if WINDOWS 
	//HANDLE file;
	FILE* file;
#elif IOS || OSX
	FILE* file;
#endif
};

#if OSX
#include <CoreFoundation/CoreFoundation.h>

//Note(ray): User app needs to include core foundations need to do something about that.
 Yostr* BuildPathToAssets(MemoryArena *Partition,u32 Type);

 dir_files_result OSXGetAllFilesInDir(Yostr Path,MemoryArena *StringMem,bool recursively = false);

 read_file_result OSXReadEntireFile(char* Path);

 read_file_result OSXReadEntireFile(Yostr Path);
#endif

#if IOS
#include <mach/mach_init.h>
 Yostr* BuildPathToAssets(MemoryArena *Partition,u32 Type);
 dir_files_result IOSGetAllFilesInDir(Yostr Path,MemoryArena *StringMem);

 read_file_result IOSReadEntireFile(char* Path);
 read_file_result IOSReadEntireFile(Yostr Path);

#endif

#if WINDOWS
#include <Windows.h>
 Yostr* BuildPathToAssets(MemoryArena* Partition, u32 Type);
 dir_files_result Win32GetAllFilesInDir(string Path, MemoryArena* StringMem);

 read_file_result Win32ReadEntireFile(char* path);k


#endif
static bool Win32WriteToFile(FILE* file, void* mem, memory_index size, bool is_done = false);

 bool PlatformWriteMemoryToFile(PlatformFilePointer* file,char* file_name,void* mem,memory_index size,bool is_done = false,char* options = "wb");

 read_file_result PlatformReadEntireFile(char* FileName);

 read_file_result PlatformReadEntireFile(Yostr* FileName);

 read_file_result PlatformReadEntireFileWithAssets(char* FileName, u32 Type, MemoryArena* Memory);

 read_file_result PlatformReadEntireFileWithAssets(Yostr* FileName, u32 Type, MemoryArena* Memory);

dir_files_result PlatformGetAllFilesInDir(Yostr Path, MemoryArena* StringMem,bool recursively = false,bool get_full_path = false);

dir_files_result PlatformGetAllAssetFilesInDir(u32 Type, MemoryArena* StringMem,bool recursively = false,bool get_full_path = false);

#ifdef YOYOIMPL

namespace APIFileOptions {
    char* data_dir = "/";
}
#if OSX
#include <CoreFoundation/CoreFoundation.h>
//Note(ray): User app needs to include core foundations need to do something about that.
 Yostr* BuildPathToAssets(MemoryArena *Partition,u32 Type)
{
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
//TODO(ray):Verify this is big enough. never big enough
    u32 DirMaxSize = 1000;
    Yostr* CurrentDir = AllocatEmptyString(Partition);
    PushSize(Partition,DirMaxSize);
    if (!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (u8 *)CurrentDir->String, DirMaxSize))
    {
// error
        Assert(false);
    }
    String_GetLength_String(CurrentDir);
    return AppendString(*CurrentDir,CreateStringFromLiteral(APIFileOptions::data_dir, Partition),Partition);
}

dir_files_result OSXGetAllFilesInDir(Yostr Path,MemoryArena *StringMem,bool recursively,bool get_full_path)
{
    dir_files_result Result;
    Result.Files = CreateVector(1000,sizeof(file_info));
    
    char* WildCard = "\\*";
    Yostr* WildCardPath = AppendString(Path, CreateStringFromLiteral(WildCard,StringMem), StringMem);
    CFAllocatorRef alloc = CFAllocatorGetDefault();
    char* Dir = Path.String;
    CFStringRef DirRef = CFStringCreateWithCString(alloc, Path.String, kCFStringEncodingASCII);
    CFURLRef UrlRef = CFURLCreateWithString(alloc, DirRef, NULL);
    CFURLEnumeratorOptions options = kCFURLEnumeratorDefaultBehavior;
    if(recursively)
    {
        options = kCFURLEnumeratorDescendRecursively;
    }
    
    CFURLEnumeratorRef Enumerator = CFURLEnumeratorCreateForDirectoryURL(alloc, UrlRef,options , 0);
    CFURLRef URL = NULL;
    while (CFURLEnumeratorGetNextURL(Enumerator, &URL, NULL) == kCFURLEnumeratorSuccess)
    {
        if(recursively && CFURLHasDirectoryPath(URL))
        {
            continue;
        }

        CFNumberRef valueNum = NULL;
        CFMutableStringRef fileName =  CFStringCreateMutableCopy(kCFAllocatorDefault, 0, CFURLGetString(URL));
        const char *cs = CFStringGetCStringPtr( fileName, kCFStringEncodingMacRoman ) ;
        Yostr* PathToFile = CreateStringFromLiteral((char*)cs, StringMem);
        Yostr* FileName;
        if(get_full_path)
        {
            FileName = PathToFile;
            //remove file prefex
            while(*(FileName->String) != '/')//after we get past the file:/than we are ready
            {
                ++FileName->String;
                FileName->Length--;
            }
        }
        else
        {
            *FileName = GetFilenameFromPath(PathToFile,StringMem);          
        }

        file_info Info;
        Info.Name = *FileName;
        PushVectorElement(&Result.Files, &Info);
        if (CFURLCopyResourcePropertyForKey(URL, kCFURLFileSizeKey, &valueNum, 0) && (valueNum != NULL))
        {
        }
        CFRelease(valueNum);
        CFRelease(fileName);
    }
    //CFRelease(URL);
    CFRelease(Enumerator);
    CFRelease(DirRef);
    CFRelease(UrlRef);
    return Result;
}

read_file_result OSXReadEntireFile(char* Path)
{
    read_file_result Result = {};
    FILE *File = fopen (Path, "r");
    if (File == NULL)
    {
        //Assert(File);
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

read_file_result OSXReadEntireFile(Yostr Path)
{
    return OSXReadEntireFile(Path.String);
}
#endif

#if IOS
#include <mach/mach_init.h>
#include <CoreFoundation/CoreFoundation.h>
//Note(ray): User app needs to include core foundations need to do something about that.
Yostr* BuildPathToAssets(MemoryArena *Partition,u32 Type)
{
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
//TODO(ray):Verify this is big enough.
    u32 DirMaxSize = 1000;
    Yostr CurrentDir = AllocateEmptyString(Partition);
    PushSize(Partition,DirMaxSize);
    if (!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (u8 *)CurrentDir.String, DirMaxSize))
    {
    }
    CalculateStringLength(&CurrentDir);
    return AppendString(CurrentDir,CreateStringFromLiteral("/data/", Partition),Partition);
}

 dir_files_result
IOSGetAllFilesInDir(Yostr Path,MemoryArena *StringMem)
{
    dir_files_result Result;
    Result.Files = CreateVector(1000,sizeof(file_info));
    
    char* WildCard = "\\*";
    Yostr* WildCardPath = AppendString(Path, CreateStringFromLiteral(WildCard,StringMem), StringMem);
    CFAllocatorRef alloc = CFAllocatorGetDefault();
    char* Dir = Path.String;
    CFStringRef DirRef = CFStringCreateWithCString(alloc, Path.String, kCFStringEncodingASCII);
    CFURLRef UrlRef = CFURLCreateWithString(alloc, DirRef, NULL);
    CFURLEnumeratorRef Enumerator = CFURLEnumeratorCreateForDirectoryURL(alloc, UrlRef, kCFURLEnumeratorDefaultBehavior, 0);
    CFURLRef URL = NULL;

    while (CFURLEnumeratorGetNextURL(Enumerator, &URL, NULL) == kCFURLEnumeratorSuccess)
    {
        CFNumberRef valueNum = NULL;
        CFMutableStringRef fileName =  CFStringCreateMutableCopy(kCFAllocatorDefault, 0, CFURLGetString(URL));
        const char *cs = CFStringGetCStringPtr( fileName, kCFStringEncodingMacRoman ) ;
        Yostr PathToFile = CreateStringFromLiteral((char*)cs, StringMem);
        char* End = PathToFile.String + PathToFile.Length - 1;
        u32 StepCount = 1;
        while(*(End - 1) != '/')
        {
            --End;
            ++StepCount;
        }
        Yostr* FileName = CreateStringFromLength(End, StepCount, StringMem);
        file_info Info;
        Info.Name = *FileName;// CreateStringFromLiteral(FileName,StringMem);// ffd.cFileName;
        PushVectorElement(&Result.Files, &Info);
        if (CFURLCopyResourcePropertyForKey(URL, kCFURLFileSizeKey, &valueNum, nullptr) && (valueNum != NULL))
        {
        }
    }
    return Result;
}

read_file_result IOSReadEntireFile(char* Path)
{
    read_file_result Result;
    FILE *File = fopen (Path, "r");
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
        Result.ContentSize = (s32)FileSize;
        void* address = PlatformAllocateMemory(FileSize);
        Result.Content = (void*)address;//allocl mem;
        fread(Result.Content, Result.ContentSize, 1, File);
        fclose(File);
    }
    return Result;
}
read_file_result IOSReadEntireFile(Yostr Path)
{
    return IOSReadEntireFile(Path.String);
}
#endif

#if WINDOWS
#include <Windows.h>


 string* BuildPathToAssets(MemoryArena* Partition, u32 Type)
{
	string* DataPath = CreateStringFromLiteral(DataDir, Partition);
	string* FinalPath;
	if (Type == 0)
	{
		FinalPath = AppendString(*DataPath, CreateStringFromLiteral("/", Partition), Partition);
	}
	else if (Type == 1)
	{
		FinalPath = AppendString(*DataPath, CreateStringFromLiteral("material/", Partition), Partition);
	}
	else if (Type == 2)
	{
		FinalPath = AppendString(*DataPath, CreateStringFromLiteral("shaders/", Partition), Partition);
	}
	else if (Type == 3)
	{
		FinalPath = AppendString(*DataPath, CreateStringFromLiteral("textures/", Partition), Partition);
	}
	else if (Type == 4)
	{
		FinalPath = AppendString(*DataPath, CreateStringFromLiteral("models/", Partition), Partition);
	}
	else if (Type == 5)
	{
		FinalPath = AppendString(*DataPath, CreateStringFromLiteral("textures/", Partition), Partition);
	}
	else if (Type == 6)
	{
		FinalPath = AppendString(*DataPath, CreateStringFromLiteral("lighting/", Partition), Partition);
	}

	string* CurrentDir = AllocatEmptyString(Partition);
	u32 Size = GetCurrentDirectory(0,NULL);
	PushSize(Partition,Size);
	GetCurrentDirectory(Size, CurrentDir->String);
	CurrentDir->Length = Size;
	{
		//TODO(ray):Some error handling.
	}
	return AppendString(*CurrentDir, *FinalPath, Partition);;
}

dir_files_result Win32GetAllFilesInDir(string Path, MemoryArena* StringMem)
{
	dir_files_result Result;

	char* WildCard = "\\*";
	string* WildCardPath = AppendString(Path, *CreateStringFromLiteral(WildCard, StringMem), StringMem);

	Result.Files = CreateVector(1000, sizeof(file_info));
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
			Info.Name = CreateStringFromLiteral(ffd.cFileName, StringMem);// ffd.cFileName;
			PushVectorElement(&Result.Files, &Info);

			//_tprintf(TEXT("  %s   %ld bytes\n"), ffd.cFileName, filesize.QuadPart);
		}
	}
	while (FindNextFile(hFind, &ffd) != 0);
	return Result;
}


read_file_result Win32ReadEntireFile(char* path)
{
	//Assert(Path);
	read_file_result Result = {};

	HANDLE File = CreateFileA(
		path,
		GENERIC_READ,
		FILE_SHARE_READ,
		nullptr,
		OPEN_EXISTING,
		0,
		nullptr
	);

	if (File != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER FileSize;
		if (GetFileSizeEx(
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
			if (Result.Content)
			{
				if (ReadFile(
					File,
					Result.Content,
					FileSize.QuadPart,
					&SizeResult,
					0
				))
				{
					Result.ContentSize = FileSize.QuadPart;
				}
				else
				{
					//TODO(ray):logging
				}
			}
			else
			{
				//TODO(ray):logging
				Win32DeAllocateMemory(Result.Content, Result.ContentSize);
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
static bool Win32WriteToFile(FILE* file, void* mem, memory_index size, bool is_done )
{
    bool result = false;
    fwrite(mem, size, 1, file);
    if (ferror(file))
    {
        result = false;
    }
    else
    {
        result = true;
    }
    if(is_done)
    {
        fclose(file);
    }
    return result;
}

 bool PlatformWriteMemoryToFile(PlatformFilePointer* file,char* file_name,void* mem,memory_index size,bool is_done,char* options)
{
#if WINDOWS 
	if(file->file == nullptr)
	{
		file->file = fopen(file_name, options);
		//file->file = CreateFileA(file_name, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	}
	return Win32WriteToFile(file->file,mem, size,is_done);
#elif IOS | OSX

    if(file->file == nullptr)
    {
        file->file = fopen(file_name, options);
        printf( "Error opening file: %s\n", strerror( errno ) );
        //file->file = CreateFileA(file_name, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    }
    return Win32WriteToFile(file->file,mem, size,is_done);
    
#endif
}

read_file_result PlatformReadEntireFile(char* FileName)
{
	read_file_result Result;
#if WINDOWS
	Result = Win32ReadEntireFile(FileName);
#elif OSX
	Result = OSXReadEntireFile(FileName);
#elif IOS
	Result = IOSReadEntireFile(FileName);
#endif
	return Result;
}

read_file_result PlatformReadEntireFile(Yostr* FileName)
{
	read_file_result Result;
#if WINDOWS
	Result = Win32ReadEntireFile((*FileName).String);
#elif OSX
    Result = OSXReadEntireFile((*FileName).String);
#elif IOS
    Result = IOSReadEntireFile((*FileName).String);
#endif
	return Result;
}

read_file_result PlatformReadEntireFileWithAssets(char* FileName, u32 Type, MemoryArena* Memory)
{
	read_file_result Result;
#if WINDOWS
	string* AssetPath = BuildPathToAssets(Memory, Type);
	string* FinalPathToAsset = AppendString(*AssetPath, CreateStringFromLiteral(FileName, Memory), Memory);

	Result = Win32ReadEntireFile(FinalPathToAsset->String);

#elif OSX
	Yostr* AssetPath = BuildPathToAssets(Memory, Type);
	Yostr* FinalPathToAsset = AppendString(*AssetPath, CreateStringFromLiteral(FileName, Memory), Memory);

	Result = OSXReadEntireFile(*FinalPathToAsset);

#elif IOS
	Yostr* AssetPath = BuildPathToAssets(Memory,Type);
	Yostr* FinalPathToAsset = AppendString(*AssetPath, CreateStringFromLiteral(FileName, Memory), Memory);

	Result = IOSReadEntireFile(*FinalPathToAsset);
#endif
	return Result;
}

read_file_result PlatformReadEntireFileWithAssets(Yostr* FileName, u32 Type, MemoryArena* Memory)
{
	read_file_result Result;
#if WINDOWS
	string* AssetPath = BuildPathToAssets(Memory, Type);
	string* FinalPathToAsset = AppendString(*AssetPath, CreateStringFromLiteral(FileName->String, Memory), Memory);

	Result = Win32ReadEntireFile(FinalPathToAsset->String);

#elif OSX
    Yostr* AssetPath = BuildPathToAssets(Memory,Type);
    Yostr* FinalPathToAsset = AppendString(*AssetPath,CreateStringFromLiteral(FileName->String,Memory),Memory);
    Result = OSXReadEntireFile(*FinalPathToAsset);
    
#elif IOS
    Yostr* AssetPath = BuildPathToAssets(Memory,Type);
    Yostr* FinalPathToAsset = AppendString(*AssetPath,*FileName,Memory);

    Result = IOSReadEntireFile(*FinalPathToAsset);
#endif
	return Result;
}

dir_files_result PlatformGetAllFilesInDir(Yostr Path, MemoryArena* StringMem,bool recursively,bool get_full_path)
{
	dir_files_result Result;
#if WINDOWS
	Result = Win32GetAllFilesInDir(Path, StringMem);
#elif OSX
    Result = OSXGetAllFilesInDir(Path, StringMem,recursively,get_full_path);
#elif IOS
    Result = IOSGetAllFilesInDir(Path, StringMem);
#endif
	return Result;
}

dir_files_result PlatformGetAllAssetFilesInDir(u32 Type, MemoryArena* StringMem,bool recursively,bool get_full_path)
{
	dir_files_result Result;

	Yostr* Path = BuildPathToAssets(StringMem,Type);

#if WINDOWS
	Result = Win32GetAllFilesInDir(*Path, StringMem);
#elif OSX
    Result = OSXGetAllFilesInDir(*Path, StringMem,recursively,get_full_path);
#elif IOS
    Result = IOSGetAllFilesInDir(*Path, StringMem);
#endif
	return Result;
}

/*
------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2018 Ray Garner
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

#define API_FILE_H
#endif
