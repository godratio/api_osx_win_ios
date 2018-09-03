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

static char* DataDir = "/../../data/";

struct YoyoReadFileResult
{
	s32 content_size;
	void* content;
};

struct YoyoDirFilesResult
{
	YoyoVector files;//files infos
	~YoyoDirFilesResult()
	{
		FreeVectorMem(&files);
	}
};

struct YoyoFileInfo
{
	void* file;
	YoyoAString* name;
	memory_index file_size;
	u32 file_count;
};

#if OSX
#include <CoreFoundation/CoreFoundation.h>


//Note(ray): User app needs to include core foundations need to do something about that.
static string* BuildPathToAssets(MemoryArena *Partition,u32 Type)
{
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
    String_GetLength_String(CurrentDir);
    return AppendString(*CurrentDir,*CreateStringFromLiteral("/", Partition),Partition);
}

static dir_files_result
OSXGetAllFilesInDir(string Path,MemoryArena *StringMem)
{
    dir_files_result Result;
    Result.Files = CreateVector(1000,sizeof(file_info));
    
    char* WildCard = "\\*";
    string* WildCardPath = AppendString(Path, *CreateStringFromLiteral(WildCard,StringMem), StringMem);
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
        Info.Name = FileName;
        PushVectorElement(&Result.Files, &Info);
        if (CFURLCopyResourcePropertyForKey(URL, kCFURLFileSizeKey, &valueNum, 0) && (valueNum != NULL))
        {
        }
    }
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

#if IOS
#include <mach/mach_init.h>
//Note(ray): User app needs to include core foundations need to do something about that.
static string* BuildPathToAssets(memory_partition *Partition)
{
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
//TODO(ray):Verify this is big enough.
    u32 DirMaxSize = 1000;
    string* CurrentDir = AllocatEmptyString(Partition);
    PushSize(Partition,DirMaxSize);
    if (!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (u8 *)CurrentDir->String, DirMaxSize))
    {
    }
    CalculateStringLength(CurrentDir);
    return AppendString(*CurrentDir,*CreateStringFromLiteral("/", Partition),Partition);
}

static dir_files_result
IOSGetAllFilesInDir(string Path,memory_partition *StringMem)
{
    dir_files_result Result;
    Result.Files = CreateVector(1000,sizeof(file_info));
    
    char* WildCard = "\\*";
    string* WildCardPath = AppendString(Path, *CreateStringFromLiteral(WildCard,StringMem), StringMem);
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
        if (CFURLCopyResourcePropertyForKey(URL, kCFURLFileSizeKey, &valueNum, nullptr) && (valueNum != NULL))
        {
        }
    }
    return Result;
}

static read_file_result
IOSReadEntireFile(string Path)
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
        Result.ContentSize = (s32)FileSize;
        void* address = PlatformAllocateMemory(FileSize);
        Result.Content = (void*)address;//allocl mem;
        fread(Result.Content, Result.ContentSize, 1, File);
        fclose(File);
    }
    return Result;
}

#endif

#if WINDOWS
#include <Windows.h>

enum YoyoDirectoryType
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

static YoyoAString* YoyoBuildPathToAssets(MemoryArena* arena, u32 type)
{
	YoyoAString* data_path = CreateStringFromLiteral(DataDir, arena);
	YoyoAString* final_path;
	if (type == 0)
	{
		final_path = AppendString(*data_path, *CreateStringFromLiteral("/", arena), arena);
	}
	else if (type == 1)
	{
		final_path = AppendString(*data_path, *CreateStringFromLiteral("material/", arena), arena);
	}
	else if (type == 2)
	{
		final_path = AppendString(*data_path, *CreateStringFromLiteral("shaders/", arena), arena);
	}
	else if (type == 3)
	{
		final_path = AppendString(*data_path, *CreateStringFromLiteral("textures/", arena), arena);
	}
	else if (type == 4)
	{
		final_path = AppendString(*data_path, *CreateStringFromLiteral("models/", arena), arena);
	}
	else if (type == 5)
	{
		final_path = AppendString(*data_path, *CreateStringFromLiteral("textures/", arena), arena);
	}
	else if (type == 6)
	{
		final_path = AppendString(*data_path, *CreateStringFromLiteral("lighting/", arena), arena);
	}

	YoyoAString* current_dir = AllocatEmptyString(arena);
	u32 size = GetCurrentDirectory(0,NULL);
	PushSize(arena,size);
	GetCurrentDirectory(size, current_dir->string);
	current_dir->length = size;
	{
		//TODO(ray):Some error handling.
	}
	return YoyoAsciiAppendString(*current_dir, *final_path, arena);;
}

static YoyoDirFilesResult
YoyoWin32GetAllFilesInDir(YoyoAString path, MemoryArena* string_mem)
{
	YoyoDirFilesResult result;

	char* wild_card = "\\*";
	YoyoAString* wild_card_path = AppendString(path, *CreateStringFromLiteral(wild_card, string_mem), string_mem);

	result.files = YoyoInitVector(1000, sizeof(YoyoFileInfo));
	WIN32_FIND_DATA ffd;
	TCHAR szDir[MAX_PATH];
	HANDLE hFind = INVALID_HANDLE_VALUE;
	LARGE_INTEGER filesize;

	hFind = FindFirstFile(wild_card_path->string, &ffd);

	if (INVALID_HANDLE_VALUE == hFind)
	{
		return result;
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

			YoyoFileInfo info;
			info.name = CreateStringFromLiteral(ffd.cFileName, string_mem);// ffd.cFileName;
			YoyoPushBack(&result.files, &info);

			//_tprintf(TEXT("  %s   %ld bytes\n"), ffd.cFileName, filesize.QuadPart);
		}
	}
	while (FindNextFile(hFind, &ffd) != 0);
	return result;
}

static YoyoReadFileResult
YoyoWin32ReadEntireFile(char* path)
{
	//Assert(Path);
	YoyoReadFileResult result = {};

	HANDLE file = CreateFileA(
		path,
		GENERIC_READ,
		FILE_SHARE_READ,
		nullptr,
		OPEN_EXISTING,
		0,
		nullptr
	);

	if (file != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER file_size;
		if (GetFileSizeEx(
			file,
			&file_size
		))
		{
			DWORD size_result;
			result.content = VirtualAlloc(
				0,
				file_size.QuadPart,
				MEM_COMMIT | MEM_RESERVE,
				PAGE_READWRITE
			);
			if (result.content)
			{
				if (ReadFile(
					file,
					result.content,
					file_size.QuadPart,
					&size_result,
					0
				))
				{
					result.content_size = file_size.QuadPart;
				}
				else
				{
					//TODO(ray):logging
				}
			}
			else
			{
				//TODO(ray):logging
				Win32DeAllocateMemory(result.content, result.content_size);
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
	CloseHandle(file);
	return result;
}

internal bool YoyoWin32WriteToFile(FILE* file, void* mem, memory_index size, bool is_done = false)
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

#endif

struct YoyoPlatformFilePointer
{
#if WINDOWS 
	//HANDLE file;
	FILE* file;
#elif IOS || OSX
	FILE file;
#endif
};

static bool YoyoPlatformWriteMemoryToFile(YoyoPlatformFilePointer* file,char* file_name,void* mem,memory_index size,bool is_done = false)
{
#if WINDOWS 
	if(file->file == nullptr)
	{
		file->file = fopen(file_name, "wb");
	}
	return YoyoWin32WriteToFile(file->file,mem, size,is_done);
#elif IOS | OSX
	Assert(false);
#endif
}

static YoyoReadFileResult YoyoPlatformReadEntireFile(char* FileName)
{
	YoyoReadFileResult result;
#if WINDOWS
	result = YoyoWin32ReadEntireFile(FileName);
#elif OSX
	Result = OSXReadEntireFile(FileName);
#elif IOS
	Result = IOSReadEntireFile(FileName);
#endif
	return result;
}

static YoyoReadFileResult YoyoPlatformReadEntireFile(YoyoAString* FileName)
{
	YoyoReadFileResult result;
#if WINDOWS
	result = YoyoWin32ReadEntireFile(YoyoAsciiNullTerminate(*FileName).string);
#elif OSX
    Result = OSXReadEntireFile(NullTerminate(*FileName));
#elif IOS
    Result = IOSReadEntireFile(NullTerminate(*FileName));
#endif
	return result;
}

static YoyoReadFileResult YoyoPlatformReadEntireFileFromAssets(char* FileName, u32 Type, MemoryArena* Memory)
{
	YoyoReadFileResult result;
#if WINDOWS
	YoyoAString* asset_path = YoyoBuildPathToAssets(Memory, Type);
	YoyoAString* final_path_to_asset = AppendString(*asset_path, *CreateStringFromLiteral(FileName, Memory), Memory);
	*final_path_to_asset = YoyoAsciiNullTerminate(*final_path_to_asset);
	result = YoyoWin32ReadEntireFile(final_path_to_asset->string);

#elif OSX
	string* AssetPath = BuildPathToAssets(Memory, Type);
	string* FinalPathToAsset = AppendString(*AssetPath, *CreateStringFromLiteral(FileName, Memory), Memory);
	NullTerminate(*FinalPathToAsset);
	Result = OSXReadEntireFile(*FinalPathToAsset);

#elif IOS
	string* AssetPath = BuildPathToAssets(Memory);
	string* FinalPathToAsset = AppendString(*AssetPath, *FileName, Memory);
	NullTerminate(*FinalPathToAsset);
	Result = IOSReadEntireFile(*FinalPathToAsset);
#endif
	return result;
}

static YoyoReadFileResult YoyoPlatformReadEntireFileFromAssets(YoyoAString* FileName, u32 Type, MemoryArena* Memory)
{
	YoyoReadFileResult result;
#if WINDOWS
	YoyoAString* asset_path = YoyoBuildPathToAssets(Memory, Type);
	YoyoAString* final_path_to_asset = AppendString(*asset_path, *CreateStringFromLiteral(FileName->string, Memory), Memory);
	*final_path_to_asset = YoyoAsciiNullTerminate(*final_path_to_asset);
	result = YoyoWin32ReadEntireFile(final_path_to_asset->string);

#elif OSX
    string* AssetPath = BuildPathToAssets(Memory,Type);
    string* FinalPathToAsset = AppendString(*AssetPath,*CreateStringFromLiteral(FileName->String,Memory),Memory);
    NullTerminate(*FinalPathToAsset);
    Result = OSXReadEntireFile(*FinalPathToAsset);
    
#elif IOS
    string* AssetPath = BuildPathToAssets(Memory);
    string* FinalPathToAsset = AppendString(*AssetPath,*FileName,Memory);
    NullTerminate(*FinalPathToAsset);
    Result = IOSReadEntireFile(*FinalPathToAsset);
#endif
	return result;
}

static YoyoDirFilesResult YoyoPlatformGetAllFilesInDir(YoyoAString Path, MemoryArena* StringMem)
{
	YoyoDirFilesResult result;
#if WINDOWS
	result = YoyoWin32GetAllFilesInDir(Path, StringMem);
#elif OSX
    Result = OSXGetAllFilesInDir(Path, StringMem);
#elif IOS
    Result = IOSGetAllFilesInDir(Path, StringMem);
#endif
	return result;
}

static YoyoDirFilesResult YoyoPlatformGetAllAssetFilesInDir(u32 Type, MemoryArena* StringMem)
{
	YoyoDirFilesResult result;

	YoyoAString* path = YoyoBuildPathToAssets(StringMem, Type);

#if WINDOWS
	result = YoyoWin32GetAllFilesInDir(*path, StringMem);
#elif OSX
    Result = OSXGetAllFilesInDir(*Path, StringMem);
#elif IOS
    Result = IOSGetAllFilesInDir(*Path, StringMem);
#endif
	return result;
}

#define API_FILE_H
#endif

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
