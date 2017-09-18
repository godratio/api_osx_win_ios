#if !defined(API_INCLUDE_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Ray Garner $
   ======================================================================== */
#include "types.h"

//NOTE(ray): This is how we expose all our libs
#ifdef API_STRING_STATIC
#define APIDEF static
#else
#ifdef __cplusplus
#define APIDEF extern "C"
#else
#define APIDEF extern
#endif
#endif

#ifndef _MSC_VER
#ifdef __cplusplus
#define api__inline inline
#else
#define api__inline
#endif
#else
#define api__inline __forceinline
#endif

enum var_type
{
    var_type_string,
    var_type_char,
    var_type_f64,
    var_type_f32,
    var_type_u64,
    var_type_u32,
    var_type_u16,
    var_type_u8,
    var_type_s64,
    var_type_s32,
    var_type_s16,
    var_type_s8,
    var_type_b32,
    var_type_memory_index,//REname this
    
};

#include "api_strings.h"
#include "api_memory.h"
#include "api_parse.h"
#include "api_tokenizer.h"
#include "api_vector.h"
#include "api_file.h"
#include "api_meta.h"

#define API_INCLUDE_H
#endif
