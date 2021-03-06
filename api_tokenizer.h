#if !defined(API_TOKENIZER_H)

#include "../yoyo/yoyoyo_ascii_vector.h"

enum token_type
{
    Token_Identifier,
    Token_Paren,
    Token_OpenParen,
    Token_CloseParen,
    Token_Asterisk,
    Token_OpenBrace,
    Token_CloseBrace,
    Token_String,
    Token_SemiColon,
    Token_Colon,
    Token_Period,
    Token_Dash,
    Token_Underscore,
    Token_Comma,
    Token_EndOfStream,
    Token_Comment,
    Token_ReturnCarriage,
    Token_NewLine,
	Token_ForwardSlash,
	Token_BackwardSlash,
    Token_Unknown,
    
};

struct token
{
    token_type Type;
    Yostr Data;
};

struct tokenizer
{
    token *LastToken;
    char* At;
};

struct csv_field
{
    Yostr Text;
};

struct csv_line
{
    YoyoVector Fields;
};

struct csv_data
{
    YoyoVector Lines;
};


struct cfg_entry
{
    b32 IsDef;
    Yostr Key;
    Yostr Text;
    var_type Type;
};

struct cfg_block
{
    b32 IsDef;
    Yostr Name;
    YoyoVector Entries;
};

struct cfg_data
{
    YoyoVector Blocks;
};

 b32 IsWhiteSpace(char At);

b32 IsWhiteSpaceNoEndOfLine(char At);

b32 IsNewLine(char At);

b32 IsSingleLineCommentCPPStyle(char* At);

b32 IsSingleLineCommentLispStyle(char* At);

b32 IsMultiLineCommentCStyle(char* At);

b32 IsMultiLineCommentCPPStyleEnd(char* At);

b32 IsDoubleDash(char* At);

b32 RequireToken(token Token,token_type TokenType);

b32 MatchToken(token Token,Yostr* Test);

b32 IsAlpha(char At);

b32 IsNum(char At);

void ParseSingleLineCommentCPPStyle(tokenizer* Tokenizer);

void ParseMultLineCommentCStyle(tokenizer* Tokenizer);

void EatAllWhiteSpace(tokenizer *Tokenizer,b32 IncludeEndOfLineChars = false);

token GetToken(tokenizer *Tokenizer,MemoryArena* Partition);

//Note(ray): Has a lot of special cases retrieving tokens mostly for.
//scripting like language and fbx ascii.
//TODO(ray): Eventually would like remove this and have anyone who is 
//use the token api to just handle the special cases for their use case.
//ie get raw tokens above and handle lexing of the text for each case seperately.
token GetCSVToken(tokenizer *Tokenizer,MemoryArena* Partition);


//NOTE(ray):Output will be a vector
csv_data ParseCSV(MemoryArena Memory, char* TextString,u32 FieldCount);
 token GetCFGToken(tokenizer *Tokenizer, MemoryArena* Partition);
 void ParseConfigKeyValues(cfg_block* Block,tokenizer* Tokenizer,MemoryArena* Memory);
#define MAX_BLOCKS 100
 void ParseConfigBlock(cfg_data* Data,tokenizer* Tokenizer,token NameToken,MemoryArena *Memory);
 void ParseDefBlock(cfg_data* Data,tokenizer* Tokenizer,MemoryArena *Memory);
 cfg_data ParseConfig(MemoryArena *Memory, char* TextString);
 token GetUIToken(tokenizer *Tokenizer, MemoryArena* Partition);
 token GetSeedToken(tokenizer *Tokenizer, MemoryArena* Partition);

#ifdef YOYOIMPL

YoyoVector Tokens;

 b32 IsWhiteSpace(char At)
{
    if (At == ' '  ||
        At == '\t' ||
        At == '\n' ||
        At == '\r' )
    {
        return true;
    }
    return false;
}

 b32 IsWhiteSpaceNoEndOfLine(char At)
{
    if (At == ' '  ||
        At == '\t')
    {
        return true;
    }
    return false;
}

 b32 IsNewLine(char At)
{
    return (At == '\n' || At == '\r');
}

 b32 IsSingleLineCommentCPPStyle(char* At)
{
    return (At[0] == '/' && At[1] == '/');
}

 b32 IsSingleLineCommentLispStyle(char* At)
{
    return (At[0] == ';');
}

 b32 IsMultiLineCommentCStyle(char* At)
{
    return (At[0] == '/' && At[1] == '*');
}
 b32 IsMultiLineCommentCPPStyleEnd(char* At)
{
    return (At[0] == '*' && At[1] == '/');
}
 b32 IsDoubleDash(char* At)
{
    return (At[0] == '-' && At[1] == '-');
}

 b32 RequireToken(token Token,token_type TokenType)
{
    
    return (Token.Type == TokenType);
}

 b32 MatchToken(token Token,Yostr* Test)
{
    if(Compare(Token.Data,*Test))
    {
        return true;
    }
    return false;
}

 b32 IsAlpha(char At)
{
    b32 Result = ((At >= 'a' && At <= 'z') ||
                  (At >= 'A' && At <= 'Z'));
    return Result;
}

 b32 IsNum(char At)
{
    return (At >= '0' && At <= '9') || (At == '-') || (At == '.');
}
 void ParseSingleLineCommentCPPStyle(tokenizer* Tokenizer)
{
    for(;;)
    {
        if(IsNewLine(*Tokenizer->At))
        {
            return;
        }
        else
        {
            ++Tokenizer->At;
        }
        
    }
}

 void ParseMultLineCommentCStyle(tokenizer* Tokenizer)
{
    for(;;)
    {
        if(IsMultiLineCommentCPPStyleEnd(Tokenizer->At))
        {
            Tokenizer->At++;
            return;
        }
        else
        {
            ++Tokenizer->At;
        }
    }
}

 void EatAllWhiteSpace(tokenizer *Tokenizer,b32 IncludeEndOfLineChars)
{
    if(IncludeEndOfLineChars)
    {
        while (IsWhiteSpace(*Tokenizer->At))
        {
            ++Tokenizer->At;
        }
        
    }
    else
    {
        while (IsWhiteSpaceNoEndOfLine(*Tokenizer->At))
        {
            ++Tokenizer->At;
        }
        
    }
}

 token 
GetToken(tokenizer *Tokenizer,MemoryArena* Partition)
{
    token Result;
    EatAllWhiteSpace(Tokenizer,true);
    
//    Yostr* TokenString;
//    token_type Type;
//    u32 TokenLength = 0;
//    char *Start = Tokenizer->At;
    b32 HasAdvanced = false;
    while (!IsWhiteSpace(*Tokenizer->At))
    {
        switch (*Tokenizer->At)
        {
            //case ';': {Result.Type = Token_SemiColon; }break;
            //NOTE(ray):This works only for fbx comments not for csv...!
            case ';': //Comments considered white space
            {
                Result.Type = Token_Comment;
                Result.Data = CreateStringFromToChar(Tokenizer->At, "\n",Partition);
                while (!IsNewLine(*Tokenizer->At++))continue;
                return Result;
            }break;
            case '(': {Result.Type = Token_OpenParen; }break;
            case ')': {Result.Type = Token_CloseParen; }break;
            case '{': {Result.Type = Token_OpenBrace; }break;
            case '}': {Result.Type = Token_CloseBrace; }break;
            case ':': {Result.Type = Token_Colon; }break;
            case ',': {Result.Type = Token_Comma; }break;
            //TODO(ray)://WHy did comment this out. investigate. probably
            //due to fbx ascii issues.
            //case '.': {Result.Type = Token_Period; }break;
            //case '-': {Result.Type = Token_Period; }break;
            case '_': {Result.Type = Token_Underscore; }break;
            //TODO(ray):Verify this doesnt screw other things up.
            case '\0': {
                Result.Type = Token_EndOfStream; 
                return Result;
            }break;
            case '"': 
            {
                Result.Type = Token_String;
                Tokenizer->At++;
                char* Start = Tokenizer->At;
                while (*(Tokenizer->At) != '"')
                {
                    Tokenizer->At++;
                    continue;
                }
                Result.Data = CreateStringFromToPointer(Start, (Tokenizer->At), Partition);
                Tokenizer->At++;
                return Result;
            }break;
            default: 
            {
                Result.Type = Token_Identifier;
                char* Start = Tokenizer->At;
                while (IsAlpha(*Tokenizer->At) || 
                       IsNum(*Tokenizer->At))
                {
                    Tokenizer->At++;
                    continue;
                }
                Result.Data = CreateStringFromToPointer(Start, Tokenizer->At, Partition);
                if(*Tokenizer->At == '\r' || *Tokenizer->At == '\n')
                {
                    //Go back one to catch the new line or return carriage on next 
                    //iteration.
                    //Tokenizer->At--;
                }
                HasAdvanced = true;
                return Result;
            }break;
        }
        if(!HasAdvanced)++Tokenizer->At;
    }
    return Result;
}

//Note(ray): Has a lot of special cases retrieving tokens mostly for.
//scripting like language and fbx ascii.
//TODO(ray): Eventually would like remove this and have anyone who is 
//use the token api to just handle the special cases for their use case.
//ie get raw tokens above and handle lexing of the text for each case seperately.
 token 
GetCSVToken(tokenizer *Tokenizer,MemoryArena* Partition)
{
    token Result;
    EatAllWhiteSpace(Tokenizer,false);
    
    while(*Tokenizer->At == ' '  ||
          *Tokenizer->At == '\t')
    {
        ++Tokenizer->At;
    }
//    Yostr* TokenString;
//    token_type Type;
//    u32 TokenLength = 0;
//    char *Start = Tokenizer->At;
    b32 HasAdvanced = false;
    while (!(*Tokenizer->At == ' '  ||
             *Tokenizer->At == '\t'))
    {
        switch (*Tokenizer->At)
        {
            //case ';': {Result.Type = Token_SemiColon; }break;
            //NOTE(ray):This works only for fbx comments not for csv...!
            case ';':  {Result.Type = Token_SemiColon; }break;
            case '(':  {Result.Type = Token_OpenParen; }break;
            case ')':  {Result.Type = Token_CloseParen; }break;
            case '{':  {Result.Type = Token_OpenBrace; }break;
            case '}':  {Result.Type = Token_CloseBrace; }break;
            case ':':  {Result.Type = Token_Colon; }break;
            case ',':  {Result.Type = Token_Comma; }break;
            case '_':  {Result.Type = Token_Underscore; }break;
			case '/':  {Result.Type = Token_ForwardSlash; }break;
			case '\\': {Result.Type = Token_BackwardSlash; }break;
			case '*':  {Result.Type = Token_Asterisk; }break;
            //TODO(ray):Verify we dont have edge cases here.
            //for now moving on.
            case '\r':
            {
                //TODO(ray):Handle possible cases.
                Result.Type = Token_ReturnCarriage;
            }break;
            case '\n':
            {
                //TODO(ray):Handle possible cases.
                Result.Type = Token_NewLine;
            }break;
            case '"': { Result.Type = Token_Paren; }break;
            case '-': { Result.Type = Token_Dash; }break;
            case '\0': 
            {
                Result.Type = Token_EndOfStream; 
                return Result;
            }break;
            default: {
                Result.Type = Token_Identifier;
                char* Start = Tokenizer->At;
                while (IsAlpha(*Tokenizer->At) || 
                       IsNum(*Tokenizer->At))
                {
                    Tokenizer->At++;
                    continue;
                }
                Result.Data = CreateStringFromToPointer(Start, Tokenizer->At, Partition);
                HasAdvanced = true;
                return Result;
            }break;
        }
        if(!HasAdvanced){++Tokenizer->At;return Result;}
    }
    return Result;
}

//NOTE(ray):Output will be a vector
 csv_data
ParseCSV(MemoryArena Memory, char* TextString,u32 FieldCount)
{
    csv_data Data;
    u32 MemSize = 10000;
    Data.Lines = YoyoInitVectorSize(MemSize, sizeof(csv_line),false);
    
    b32 IsParsing = true;
    tokenizer Tokenizer = {0};
    Tokenizer.At = TextString;
    YoyoVector TokenVector = YoyoInitVectorSize(MemSize, sizeof(token),false);
    
    //token *Token;
//    u32 LineNumber = 0;
    csv_line* CurrentLine = YoyoPushAndCastEmptyVectorElement(csv_line,&Data.Lines);
    CurrentLine->Fields = YoyoInitVectorSize(FieldCount,sizeof(csv_field),false);
    token PrevToken;
    while(IsParsing)
    {
        token Token = GetCSVToken(&Tokenizer, &Memory);
        YoyoPushBack(&TokenVector, Token);
        
        if(Token.Type == Token_Identifier)
        {
            csv_field Field;
            Field.Text = Token.Data;
            YoyoPushBack(&CurrentLine->Fields,Field);
        }
        else if(Token.Type == Token_ReturnCarriage || Token.Type == Token_NewLine)
        {
            if(PrevToken.Type != Token_ReturnCarriage)
            {
//                ++LineNumber;
                CurrentLine = YoyoPushAndCastEmptyVectorElement(csv_line,&Data.Lines);
                CurrentLine->Fields = YoyoInitVectorSize(FieldCount,sizeof(csv_field),false);
            }
        }
        
        if (Token.Type == Token_EndOfStream)
        {
            break;
        }
        PrevToken = Token;
    }
    
    YoyoFreeVectorMem(&TokenVector);
    return Data;
}

 token
GetCFGToken(tokenizer *Tokenizer, MemoryArena* Partition)
{
    token Result;
    EatAllWhiteSpace(Tokenizer);
    //Comment skipped.
    if(IsSingleLineCommentCPPStyle(Tokenizer->At))
    {
        ParseSingleLineCommentCPPStyle(Tokenizer);
    }
    if(IsMultiLineCommentCStyle(Tokenizer->At))
    {
        ParseMultLineCommentCStyle(Tokenizer);
    }
    char AtChar = *Tokenizer->At;
    ++Tokenizer->At;
        switch (AtChar)
        {
            
            case '(': {Result.Type = Token_OpenParen; }break;
            case ')': {Result.Type = Token_CloseParen; }break;
            case '{': {Result.Type = Token_OpenBrace; }break;
            case '}': {Result.Type = Token_CloseBrace; }break;
            case ':': {Result.Type = Token_Colon; }break;
            case ',': {Result.Type = Token_Comma; }break;
            case '_': {Result.Type = Token_Underscore; }break;
            case '-': 
            {
               if(Tokenizer->At[0] == '-')
               {
                   Result.Type = Token_Dash;
                   Tokenizer->At = Tokenizer->At + 1;
                   return Result;
               }
            }break;
            case '\0': {
                Result.Type = Token_EndOfStream;
                return Result;
            }break;
            case '"':
            {
                Result.Type = Token_String;
                char* Start = Tokenizer->At;
                while (Tokenizer->At[0] != '"')
                {
                    Tokenizer->At++;
                    continue;
                }
				Result.Data = CreateStringFromToPointer(Start, (Tokenizer->At), Partition);
                Tokenizer->At++;
                return Result;
            }break;
            default:
            {
                if(IsAlpha(AtChar))
                {
                    Result.Type = Token_Identifier;
                    char* Start = Tokenizer->At - 1;
                    while (IsAlpha(*Tokenizer->At) ||
                           IsNum(*Tokenizer->At))
                    {
                        Tokenizer->At++;
                        continue;
                    }
                    Result.Data = CreateStringFromToPointer(Start, Tokenizer->At, Partition);
                }
                else
                {
                    Result.Type = Token_Unknown;
                }
                return Result;
            }break;
        }
    return Result;
}

 void ParseConfigKeyValues(cfg_block* Block,tokenizer* Tokenizer,MemoryArena* Memory)
{
    token Token = GetCFGToken(Tokenizer,Memory);
    cfg_entry EntryCanidate;
    if(RequireToken(Token, Token_Identifier))
    {
        EntryCanidate.Key = Token.Data;
        Token = GetCFGToken(Tokenizer,Memory);
        if(Token.Type == Token_Colon)
        {
            Token = GetCFGToken(Tokenizer,Memory);
            if(Token.Type == Token_String)
            {
                EntryCanidate.Text = Token.Data;
                YoyoPushBack(&Block->Entries, EntryCanidate);
                ParseConfigKeyValues(Block,Tokenizer, Memory);
            }
            else
            {
            }
        }
        else
        {
            //TODO(ray):Missing semi colen breaking.
        }
    }
    else
    {
        //TODO(ray): No identifier for key.
    }
}

#define MAX_BLOCKS 100
 void ParseConfigBlock(cfg_data* Data,tokenizer* Tokenizer,token NameToken,MemoryArena *Memory)
{
    Yostr TaskName = NameToken.Data;
    
    cfg_block *Block = (cfg_block*)YoyoPushEmptyVectorElement(&Data->Blocks);
    Block->Name = TaskName;
    Block->Entries = YoyoInitVectorSize(MAX_BLOCKS, sizeof(cfg_entry),false);
    ParseConfigKeyValues(Block,Tokenizer, Memory);
    
}

 void ParseDefBlock(cfg_data* Data,tokenizer* Tokenizer,MemoryArena *Memory)
{
    token NameToken = GetCFGToken(Tokenizer,Memory);
    Yostr TaskName = NameToken.Data;
    
    cfg_block *Block = (cfg_block*)YoyoPushEmptyVectorElement(&Data->Blocks);
    Block->Name = TaskName;
    Block->IsDef = true;
    Block->Entries = YoyoInitVector(MAX_BLOCKS, sizeof(cfg_entry),false);
    ParseConfigKeyValues(Block,Tokenizer, Memory);
    
}

 cfg_data
ParseConfig(MemoryArena *Memory, char* TextString)
{
	cfg_data Data;
	u32 MemSize = 30;
	Data.Blocks = YoyoInitVector(MemSize, sizeof(cfg_block),false);
    
//    b32 IsParsing = true;
	tokenizer Tokenizer = { 0 };
	Tokenizer.At = TextString;

///	u32 LineNumber = 0;
//	token PrevToken;
    
    for (;;)
    {
        token Token = GetCFGToken(&Tokenizer, Memory);
        if(RequireToken(Token,Token_Dash))
        {
            token NextToken = GetCFGToken(&Tokenizer,Memory);
            if(RequireToken(NextToken,Token_Colon))
            {
                //Parse def block
                ParseDefBlock(&Data,&Tokenizer,Memory);
            }
            else
            {
                ParseConfigBlock(&Data,&Tokenizer,NextToken, Memory);                
            }

        }
        if (Token.Type == Token_EndOfStream)
        {
            break;
        }
    }
    return Data;
}

 token
GetUIToken(tokenizer *Tokenizer, MemoryArena* Partition)
{
    token Result;
    EatAllWhiteSpace(Tokenizer);
    //Comment skipped.
    if(IsSingleLineCommentCPPStyle(Tokenizer->At))
    {
        ParseSingleLineCommentCPPStyle(Tokenizer);
    }
    if(IsMultiLineCommentCStyle(Tokenizer->At))
    {
        ParseMultLineCommentCStyle(Tokenizer);
    }
    char AtChar = *Tokenizer->At;
    ++Tokenizer->At;
    switch (AtChar)
    {
            
        case '(': {Result.Type = Token_OpenParen; }break;
        case ')': {Result.Type = Token_CloseParen; }break;
        case '{': {Result.Type = Token_OpenBrace; }break;
        case '}': {Result.Type = Token_CloseBrace; }break;
        case ':': {Result.Type = Token_Colon; }break;
        case ',': {Result.Type = Token_Comma; }break;
        case '_': {Result.Type = Token_Underscore; }break;
        case '-':
        {
            if(Tokenizer->At[0] == '-')
            {
                Result.Type = Token_Dash;
                Tokenizer->At = Tokenizer->At + 2;
                return Result;
            }
        }break;
        case '\0': {
            Result.Type = Token_EndOfStream;
            return Result;
        }break;
        case '"':
        {
            Result.Type = Token_String;
            char* Start = Tokenizer->At;
            while (Tokenizer->At[0] != '"')
            {
                Tokenizer->At++;
                continue;
            }
            Tokenizer->At++;
            Result.Data = CreateStringFromToPointer(Start, (Tokenizer->At), Partition);
            return Result;
        }break;
        default:
        {
            if(IsAlpha(AtChar))
            {
                Result.Type = Token_Identifier;
                char* Start = Tokenizer->At - 1;
                while (IsAlpha(*Tokenizer->At) ||
                       IsNum(*Tokenizer->At))
                {
                    Tokenizer->At++;
                    continue;
                }
                Result.Data = CreateStringFromToPointer(Start, Tokenizer->At, Partition);
            }
            else
            {
                Result.Type = Token_Unknown;
            }
            
            
            return Result;
        }break;
    }
    return Result;
}

 token
GetSeedToken(tokenizer *Tokenizer, MemoryArena* Partition)
{
    token Result;
    EatAllWhiteSpace(Tokenizer);
    //Comment skipped.
    if(IsSingleLineCommentLispStyle(Tokenizer->At))
    {
        ParseSingleLineCommentCPPStyle(Tokenizer);
    }
//    if(IsMultiLineCommentCStyle(Tokenizer->At))
//    {
//        ParseMultLineCommentCStyle(Tokenizer);
//    }
    char AtChar = *Tokenizer->At;
    ++Tokenizer->At;
    switch (AtChar)
    {
        case '(': {Result.Type = Token_OpenParen; }break;
        case ')': {Result.Type = Token_CloseParen; }break;
        case '{': {Result.Type = Token_OpenBrace; }break;
        case '}': {Result.Type = Token_CloseBrace; }break;
        case ':': {Result.Type = Token_Colon; }break;
        case ',': {Result.Type = Token_Comma; }break;
        case '_': {Result.Type = Token_Underscore; }break;
/*Note(Ray):We allow dashes in the identiefiers in lispy/seed
        case '-': 
        {
            if(Tokenizer->At[0] == '-')
            {
                Result.Type = Token_Dash;
//                Tokenizer->At = Tokenizer->At + 1;
                return Result;
            }
        }break;
*/
        case '\0': {
            Result.Type = Token_EndOfStream;
            return Result;
        }break;
        case '"':
        {
            Result.Type = Token_String;
            char* Start = Tokenizer->At;
            while (Tokenizer->At[0] != '"')
            {
                Tokenizer->At++;
                continue;
            }
            Result.Data = CreateStringFromToPointer(Start, (Tokenizer->At), Partition);
            Tokenizer->At++;
            return Result;
        }break;
        default:
        {
//            if(IsAlpha(AtChar))
            {
                Result.Type = Token_Identifier;
                char* Start = Tokenizer->At - 1;
                while (IsAlpha(*Tokenizer->At) ||
                       IsNum(*Tokenizer->At))
                {
                    Tokenizer->At++;
                    continue;
                }
                Result.Data = CreateStringFromToPointer(Start, Tokenizer->At, Partition);
            }
//            else
            {
//                Result.Type = Token_Unknown;
            }
            return Result;
        }break;
    }
    return Result;
}

#endif
#define API_TOKENIZER_H
#endif
