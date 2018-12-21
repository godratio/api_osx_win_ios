#if !defined(API_META_H)

APIDEF var_type GetTypeForString(string* String);

APIDEF string* GetStringForType(var_type VarType,MemoryArena* StringPartition);

APIDEF void StartBlock(string* StringToUse,MemoryArena* StringPartition);

APIDEF void EndBlock(string* StringToUse,MemoryArena* StringPartition);

APIDEF void DeclareAndAssignVariable(string* StringToUse,string* VarName, string* Value,MemoryArena* StringPartition);

APIDEF void StartStructBlock(string* StringToUse,string StructName,MemoryArena *StringPartition);

#ifdef YOYOIMPL
char* WhiteSpaceText = " ";
char* StructText = "struct";
char* OpenBraceText = "{";
char* CloseBraceText = "}";
char* SemiColonText = ";";
char* NewLineText = "\n";
char* CTypeCharPtrText = "char*";
char* EqualsText = "=";

APIDEF var_type GetTypeForString(string* String)
{
    return var_type_string;
}

APIDEF string* GetStringForType(var_type VarType,MemoryArena* StringPartition)
{

    string* Result;
    switch(VarType)
    {
        case var_type_string:
        {
            Result = CreateStringFromLiteral("string",StringPartition);
        }break;
    }
    return Result;
    
}

APIDEF void StartBlock(string* StringToUse,MemoryArena* StringPartition)
{
    AppendCharToStringAndAdvace(StringToUse, NewLineText , StringPartition);
    AppendCharToStringAndAdvace(StringToUse, OpenBraceText , StringPartition);
    AppendCharToStringAndAdvace(StringToUse, NewLineText , StringPartition);
}

APIDEF void EndBlock(string* StringToUse,MemoryArena* StringPartition)
{
    AppendCharToStringAndAdvace(StringToUse, CloseBraceText , StringPartition);
    AppendCharToStringAndAdvace(StringToUse, NewLineText, StringPartition);
}
// 
APIDEF void DeclareAndAssignVariable(string* StringToUse,string* VarName, string* Value,MemoryArena* StringPartition)
{

    var_type VarType = GetTypeForString(Value);
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

APIDEF void StartStructBlock(string* StringToUse,string StructName,MemoryArena *StringPartition)
{
    
    AppendCharToStringAndAdvace(StringToUse, StructText , StringPartition);
    AppendCharToStringAndAdvace(StringToUse, WhiteSpaceText , StringPartition);

    AppendStringAndAdvance(StringToUse, StructName , StringPartition);

    StartBlock(StringToUse,StringPartition);
}

#endif
#define API_META_H
#endif
