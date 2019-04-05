#if !defined(TYPE_H)
//TODO(ray):remove this include
#include <stdint.h>

//NOTE(ray): These defines are just treated like funcitons so they dont need to be all caps.
//consider changing them to static functions later?
//Utils for being explict
#define KiloBytes(Value) (Value * 1024LL)
#define MegaBytes(Value) (KiloBytes(Value) * 1024LL)
#define GigaBytes(Value) (MegaBytes(Value) * 1024LL)
#define TeraBytes(Value) (GigaBytes(Value) * 1024LL)

#define U32FromPointer(Pointer) ((u32)(memory_index)(Pointer))
#define PointerFromU32(type, Value) (type *)((memory_index)Value)

typedef float r32;
typedef r32 f32;
typedef double f64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef intmax_t maxsize;
typedef intptr_t intptr;

typedef uintmax_t umaxsize;
typedef uintptr_t uintptr;

typedef u32 b32;

//TODO(Ray):Use compiler intrinsic where available ie..Clang GCC MSVC
//It seems that this may get optimized out of some compilers. also annoying warning.
#define ArrayCount(Array) sizeof(Array) / sizeof((Array)[0])

//NOTE(ray):These are helpful because static has different functionality in different contexts.
//these are to make it explicit
//#define internal static;
#define local_persist static;
#define global_variable static;

#define MAX_CONTROLLER_COUNT 2

#define Pi32 3.14159265359f
#define U16Maximum 65535
#define U32Maximum ((u32)-1)
#define Real32Maximum FLT_MAX
#define Real32Minimum -FLT_MAX
#define R32Maximum FLT_MAX
#define R32Minimum -FLT_MAX
#define f32Maximum FLT_MAX
#define f32Minimum -FLT_MAX

#define TYPE_H
#endif
