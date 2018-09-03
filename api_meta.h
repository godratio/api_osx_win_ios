/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Ray Garner $
   ======================================================================== */

#if !defined(API_META_H)

char* WhiteSpaceText = " ";
char* StructText = "struct";
char* OpenBraceText = "{";
char* CloseBraceText = "}";
char* SemiColonText = ";";
char* NewLineText = "\n";
char* CTypeCharPtrText = "char*";
char* EqualsText = "=";

APIDEF var_type GetTypeForString(YoyoAString* String)
{
    return var_type_string;
}

APIDEF YoyoAString* GetStringForType(var_type VarType,MemoryArena* StringPartition)
{

    YoyoAString* Result;
    switch(VarType)
    {
        case var_type_string:
        {
            Result = CreateStringFromLiteral("string",StringPartition);
        }break;
    }
    return Result;
    
}

APIDEF void StartBlock(YoyoAString* StringToUse,MemoryArena* StringPartition)
{
    AppendCharToStringAndAdvace(StringToUse, NewLineText , StringPartition);
    AppendCharToStringAndAdvace(StringToUse, OpenBraceText , StringPartition);
    AppendCharToStringAndAdvace(StringToUse, NewLineText , StringPartition);
}

APIDEF void EndBlock(YoyoAString* StringToUse,MemoryArena* StringPartition)
{
    AppendCharToStringAndAdvace(StringToUse, CloseBraceText , StringPartition);
    AppendCharToStringAndAdvace(StringToUse, NewLineText, StringPartition);
}
// 
APIDEF void DeclareAndAssignVariable(YoyoAString* StringToUse,YoyoAString* VarName, YoyoAString* Value,MemoryArena* StringPartition)
{

    var_type VarType = GetTypeForString(Value);
//    string* 
    AppendCharToStringAndAdvace(StringToUse, CTypeCharPtrText , StringPartition);
    AppendCharToStringAndAdvace(StringToUse, WhiteSpaceText, StringPartition);
    AppendCharToStringAndAdvace(StringToUse, VarName->string , StringPartition);
    AppendCharToStringAndAdvace(StringToUse, WhiteSpaceText , StringPartition);
    AppendCharToStringAndAdvace(StringToUse, EqualsText , StringPartition);
    AppendCharToStringAndAdvace(StringToUse, Value->string , StringPartition);
    AppendCharToStringAndAdvace(StringToUse, SemiColonText , StringPartition);
    AppendCharToStringAndAdvace(StringToUse, NewLineText , StringPartition);

}

APIDEF void StartStructBlock(YoyoAString* StringToUse,YoyoAString StructName,MemoryArena *StringPartition)
{
    
    AppendCharToStringAndAdvace(StringToUse, StructText , StringPartition);
    AppendCharToStringAndAdvace(StringToUse, WhiteSpaceText , StringPartition);

    AppendStringAndAdvance(StringToUse, StructName , StringPartition);

    StartBlock(StringToUse,StringPartition);
}

#define API_META_H
#endif
