#include <stdint.h>
#include <limits.h>
#include "api_memory.h"
#include "api_strings.h"

#if !defined(API_PARSE)

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
	int Fraction;
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

static float_basis
GetFloatBasis(memory_partition *Partition, string* FloatString)
{
	float_basis Result = {};
	//TODO(RAY):Get the sign of the number here
	s32 Sign = 1;
	s32 FracSign = 1;
	fixed_element_size_list StringResult =  SplitString(*FloatString,".",Partition,true);// StringSplitLine(Partition, FloatString->string, FloatString->length, '.');
	string* FloatCanidate;
	u32 Index = 0;
	while (FloatCanidate = ElementIterator(&StringResult))
	{
		if (Index == 0)
		{
			s32 Converted = ConvertStringToInt32(*FloatCanidate, &Sign);
			Result.Whole = Sign * Converted;
		}
		else
		{
			//NOTE(RAY):We are intentionally not using the fracsign here. To keep both whole and fractional
			//sign uniform
			s32 Converted = ConvertStringToInt32(*FloatCanidate, &FracSign);
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

	s32 Places = NumPlaces(Fraction);
	f32 NewFraction = Fraction;
	for (int Place = 0; Place < Places; ++Place)
	{
		NewFraction = NewFraction * 0.1f;
	}
	f32 TrueFraction = Whole + NewFraction;
	return TrueFraction;
}





#define API_PARSE
#endif
