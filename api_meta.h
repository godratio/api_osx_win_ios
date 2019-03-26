#if !defined(API_META_H)

APIDEF var_type GetTypeForString(Yostr* String);

APIDEF Yostr GetStringForType(var_type VarType,MemoryArena* StringPartition);

APIDEF void StartBlock(Yostr* StringToUse,MemoryArena* StringPartition);

APIDEF void EndBlock(Yostr* StringToUse,MemoryArena* StringPartition);

APIDEF void DeclareAndAssignVariable(Yostr* StringToUse,Yostr* VarName, Yostr* Value,MemoryArena* StringPartition);

APIDEF void StartStructBlock(Yostr* StringToUse,Yostr StructName,MemoryArena *StringPartition);

#ifdef YOYOIMPL
const char* WhiteSpaceText = " ";
const char* StructText = "struct";
const char* OpenBraceText = "{";
const char* CloseBraceText = "}";
const char* SemiColonText = ";";
const char* NewLineText = "\n";
const char* CTypeCharPtrText = "char*";
const char* EqualsText = "=";

APIDEF var_type GetTypeForString(Yostr* String)
{
    return var_type_string;
}

APIDEF Yostr GetStringForType(var_type VarType,MemoryArena* StringPartition)
{

    Yostr Result;
    switch(VarType)
    {
        case var_type_string:
        {
            Result = CreateStringFromLiteral("string",StringPartition);
        }break;
    }
    return Result;
    
}

APIDEF void StartBlock(Yostr* StringToUse,MemoryArena* StringPartition)
{
    AppendCharToStringAndAdvace(StringToUse, NewLineText , StringPartition);
    AppendCharToStringAndAdvace(StringToUse, OpenBraceText , StringPartition);
    AppendCharToStringAndAdvace(StringToUse, NewLineText , StringPartition);
}

APIDEF void EndBlock(Yostr* StringToUse,MemoryArena* StringPartition)
{
    AppendCharToStringAndAdvace(StringToUse, CloseBraceText , StringPartition);
    AppendCharToStringAndAdvace(StringToUse, NewLineText, StringPartition);
}
// 
APIDEF void DeclareAndAssignVariable(Yostr* StringToUse,Yostr* VarName, Yostr* Value,MemoryArena* StringPartition)
{

    //var_type VarType = GetTypeForString(Value);
//    string* 
    AppendCharToStringAndAdvace(StringToUse, CTypeCharPtrText , StringPartition);
    AppendCharToStringAndAdvace(StringToUse, WhiteSpaceText, StringPartition);
    AppendCharToStringAndAdvace(StringToUse, VarName->String , StringPartition);
    AppendCharToStringAndAdvace(StringToUse, WhiteSpaceText , StringPartition);
    AppendCharToStringAndAdvace(StringToUse, EqualsText , StringPartition);
    AppendCharToStringAndAdvace(StringToUse, Value->String , StringPartition);
    AppendCharToStringAndAdvace(StringToUse, SemiColonText , StringPartition);
    AppendCharToStringAndAdvace(StringToUse, NewLineText , StringPartition);

}

APIDEF void StartStructBlock(Yostr* StringToUse,Yostr StructName,MemoryArena *StringPartition)
{
    
    AppendCharToStringAndAdvace(StringToUse, StructText , StringPartition);
    AppendCharToStringAndAdvace(StringToUse, WhiteSpaceText , StringPartition);

    AppendStringAndAdvance(StringToUse, StructName , StringPartition);

    StartBlock(StringToUse,StringPartition);
}

#endif
#define API_META_H
#endif
