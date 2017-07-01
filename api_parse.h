/*
author: Ray Garner 
email : raygarner13@gmail.com

api_parse  - public domain misc parsing - 
                                     no warranty implied; use at your own risk
                                     
                                     
                                     LICENSE
  See end of file for license information.
  
  */


#if !defined (API_PARSE_H)

#include <stdint.h>
#include <limits.h>
#include <math.h>

inline s32
NumPlaces(s32 n)
{
    if (n < 0) n = (n == INT_MIN) ? INT_MAX : -n;
    if (n < 10) return 1;
    if (n < 100) return 2;
    if (n < 1000) return 3;
    if (n < 10000) return 4;
    if (n < 100000) return 5;
    if (n < 1000000) return 6;
    if (n < 10000000) return 7;
    if (n < 100000000) return 8;
    if (n < 1000000000) return 9;
    /*      2147483647 is 2^31-1 - add more ifs as needed
    and adjust this final return as well. */
    return 10;
}

inline s32
NumPow(s32 n)
{
    if (n < 0) n = (n == INT_MIN) ? INT_MAX : -n;
    if (n < 10) return 10;
    if (n < 100) return 100;
    if (n < 1000) return 1000;
    if (n < 10000) return 10000;
    if (n < 100000) return 100000;
    if (n < 1000000) return 1000000;
    if (n < 10000000) return 10000000;
    if (n < 100000000) return 100000000;
    if (n < 1000000000) return 1000000000;
    /*      2147483647 is 2^31-1 - add more ifs as needed
    and adjust this final return as well. */
    return 10;
}

inline f32
NumPowFrac(s32 n)
{
    if (n < 0) n = (n == INT_MIN) ? INT_MAX : -n;
    if (n < 10) return 0.1;
    if (n < 100) return 0.01;
    if (n < 1000) return 0.001;
    if (n < 10000) return 0.0001;
    if (n < 100000) return 0.00001;
    if (n < 1000000) return 0.000001;
    if (n < 10000000) return 0.0000001;
    if (n < 100000000) return 0.00000001;
    if (n < 1000000000) return 0.000000001;
    /*      2147483647 is 2^31-1 - add more ifs as needed
    and adjust this final return as well. */
    return 10;
}

struct base_string
{
    int length;
    char* string;
};


struct base_string_result
{
    base_string* result;
    int count;
};


struct float_basis
{
    int Whole;
    f32 Fraction;
};

//TODO(RAY):This does not properly trunucate a float instead it just removes the decimal point
static s32
ConvertStringToInt32(string String, s32 *Sign)
{
    s32 Result = 0;
    b32 Started = false;
    
    for (u32 Character = 0; Character < (u32)String.Length; ++Character)
    {
        if (String.String[Character] == '-' && !Started)
        {
            *Sign = -1;
            continue;
        }
        if ((String.String[Character] == ' ' || String.String[Character] == '.' ||
             String.String[Character] > '9' || String.String[Character] < '0'))continue;
        
        
        Started = true;
        Result = ((Result * 10) + String.String[Character]) - '0';
    }
    
    Result = Result * (*Sign);
    return Result;
}
//TODO(RAY):This does not properly trunucate a float instead it just removes the decimal point
static f32
ConvertStringToFraction(string String, s32 *Sign)
{
    f32 Result = 0;
    b32 Started = false;
    
    for (u32 Character = 0; Character < (u32)String.Length; ++Character)
    {
        if (String.String[Character] == '-' && !Started)
        {
            *Sign = -1;
            continue;
        }
        if ((String.String[Character] == ' ' || String.String[Character] == '.' ||
             String.String[Character] > '9' || String.String[Character] < '0'))continue;
        
        
        Started = true;
        
        Result = floorf((Result * (0.1f * (String.String[Character] - '0'))));
    }
    
    Result = Result * (*Sign);
    return Result;
}

struct float_proxy
{
    union
    {
        u32 Int;
        f32 Float;
    };
    
};

static f64 ParseFloat(string String)
{
    u32 CharIndex = 0;
    char *Str = String.String;
    u32 Length = String.Length;
    
    f64 Sign = 1.0;
    if (Str[CharIndex] == '-')
    {
        Sign = -1.0;
        CharIndex++;
    }
    else if (*Str == '+')
    {
        CharIndex++;
    }
    
    f64 Value = 0.0;
    for (; CharIndex < Length; CharIndex++) 
    {
        char Char = Str[CharIndex];
        if (!IsDigit(Char)) 
        {
            break;
        }
        s64 PreValue = Char - '0';
        Value *= 10.0;
        Value += PreValue;
    }
    
    if (Str[CharIndex] == '.')
    {
        f64 Pow10 = 10.0;
        CharIndex++;
        for (; CharIndex < Length; CharIndex++)
        {
            char Char = Str[CharIndex];
            if (!IsDigit(Char)) 
            {
                break;
            }
            Value += (Char-'0') / Pow10;
            Pow10 *= 10.0;
        }
    }
    
    f64 Frac = 0;
    f64 Scale = 1.0;
    if ((Str[CharIndex] == 'e') || (Str[CharIndex] == 'E')) 
    {
        CharIndex++;
        if (Str[CharIndex] == '-') 
        {
            Frac = 1;
            CharIndex++;
        }
        else if (Str[CharIndex] == '+')
        {
            CharIndex++;
        }
        
        u32 Exp;
        for (Exp = 0; IsDigit(Str[CharIndex]); CharIndex++) {
            Exp = Exp * 10 + (Str[CharIndex]-'0');
        }
        if (Exp > 308) Exp = 308;
        
        while (Exp >= 50) { Scale *= 1e50; Exp -= 50; }
        while (Exp >=  8) { Scale *= 1e8;  Exp -=  8; }
        while (Exp >   0) { Scale *= 10.0; Exp -=  1; }
    }
    
    f64 Result = Sign * (Frac ? (Value / Scale) : (Value * Scale));
    return Result;
}

//Note(ray):The bit operations will need to be reversed for little endian machines.
static float_basis
GetFloatBasis(memory_partition *Partition, string* FloatString)
{
    float_basis Result = {};
    //TODO(RAY):Get the sign of the number here
    s32 Sign = 1;
    
    s32 FracSign = 1;
    f32 FracResult = 0;
    s32 DecimalIndex = 0;
    for(;DecimalIndex <= 1;++DecimalIndex)
    {
        FracResult =  DecimalIndex;
    }
    
    //1.1f to float
    u32 WholeNumberExample = 1;
    u32 FractionNumberExample = 1;
    u8 SignBit = 0;
    u8 Exponent = 0;
    u8 Bias = 127;
    u32 Signifcand = 0;
    
    //Example is 276 binary radix moved to past the first bit 100010100
    //1.00010100 2^8 hence the 8 to the bias.
    //Used part is 23 bits 24 bit is explicitly assumed to be 1 as in the notation
    //1101 = 1.101 x 2^3
    
    s32 BitIndex = 0;
    float_proxy FProxy = {0};
    
    float FloatResult = 0;
    u32 LastBitSet = 0;
    //Check bits
    for(BitIndex = 0;BitIndex < 32;)
    {
        
        b32 BitValue =  (WholeNumberExample & (1 << BitIndex));
        if(BitValue)
        {
            LastBitSet = BitIndex;
        }
        
        BitIndex++;
    }
    
    u32 LastBitSetFrac = 0;
    //Check bits
    
    for(BitIndex = 0;BitIndex < 32;)
    {
        
        b32 BitValue = (FractionNumberExample & (1 << BitIndex));
        if(BitValue)
        {
            LastBitSetFrac = BitIndex;
        }
        
        BitIndex++;
    }
    
    u8 BiasFactor = Bias;
    BitIndex = 23;
    u32 BiasIndex = 0;
    float_proxy P = {0};
    P.Float = 1.1f;
    P.Float = 1.1f;
    //Set Exponent Bits
    for(;BiasIndex < 8;++BitIndex,++BiasIndex)
    {
        b32 BitValue =  (BiasFactor & (1 << BiasIndex));
        if(BitValue)
        {
            FProxy.Int |= (1 << BitIndex);
        }
        else
        {
            FProxy.Int |= (0 << BitIndex);
        }
    }
    
    
    //Set Significand bits
    b32 BiasFound = false;
    s32 SignificandIndex = LastBitSet;
    s32 FracSignificandIndex = LastBitSetFrac;
    BitIndex = 22;
    for(;BitIndex >= 0;--BitIndex,--SignificandIndex,--FracSignificandIndex)
    {
        b32 BitValue = 0;
        if(SignificandIndex >= 0)
        {
            BitValue =  (WholeNumberExample & (1 << SignificandIndex));
        }
        else
        {
            if(!BiasFound)Bias = 22 - BitIndex;
            BiasFound = true;
            BitValue =  (FractionNumberExample & (1 << FracSignificandIndex));
        }
        if(BitValue)
        {
            FProxy.Int |= (1 << BitIndex);
        }
        else
        {
            FProxy.Int |= (0 << BitIndex);
        }
    }
    
    FloatResult = FProxy.Float;
    //Exponent = Bias ;
    
    fixed_element_size_list StringResult =  SplitString(*FloatString,".",Partition,true);
    string* FloatCanidate;
    u32 Index = 0;
    while (FloatCanidate = ElementIterator(&StringResult))
    {
        if (Index == 0)
        {
            s32 Converted = ConvertStringToInt32(*FloatCanidate, &Sign);
            Result.Whole = Converted;
        }
        else
        {
            f32 Converted = ConvertStringToFraction(*FloatCanidate, &FracSign);
            Result.Fraction = Sign * Converted;
        }
        Index++;
    }
    
    return Result;
}

static f32
ParseFloat(memory_partition *Partition, string* FloatString)
{
    float_basis FloatBasis = GetFloatBasis(Partition, FloatString);
    //Combine these ints to make a floating point number
    //TODO(RAY):Handle the negative case
    //create the mantissa
    s32 Whole = FloatBasis.Whole;
    s32 Fraction = FloatBasis.Fraction;
    
    f32 Places = NumPowFrac(Fraction);
    f32 NewFraction = Fraction;
    {
        NewFraction = (NewFraction * Places);
    }
    f32 TrueFraction = Whole + NewFraction;
    return TrueFraction;
}

struct csv_field
{
    string* Name;
    string* Text;
};

struct csv_line
{
    vector Fields;
};

struct csv_data
{
    vector Lines;
};

#include "../api_tokenizer.h"
//NOTE(ray):Output will be a vector
static csv_data*
ParseCSV(memory_partition Memory, char* TextString)
{
    csv_data Data;
    u32 MemSize = 500;
    Data.Lines = CreateVector(MemSize, sizeof(csv_line));
    
    b32 IsParsing = true;
    tokenizer Tokenizer = {0};
    Tokenizer.At = TextString;
    vector TokenVector = CreateVector(MemSize, sizeof(token));
    
    while(IsParsing)
    {
        token Token = GetToken(&Tokenizer, &Memory);
        PushVectorElement(&TokenVector, &Token);
        
        if (Token.Type == Token_EndOfStream)
        {
            break;
        }
    }
    return 0;
}

#define API_PARSE_H
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