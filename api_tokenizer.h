#if !defined(API_TOKENIZER_H)
#include "api_osx_win/api_vector.h"

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
    Token_Unknown
};

struct token
{
    token_type Type;
    string* Data;
};

struct tokenizer
{
    token *LastToken;
    char* At;
};

vector Tokens;
static b32 IsWhiteSpace(char At)
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

static b32 IsNewLine(char At)
{
    return (At == '\n' || At == '\r');
}

static b32 IsAlpha(char At)
{
    b32 Result = ((At >= 'a' && At <= 'z') ||
                  (At >= 'A' && At <= 'Z'));
    return Result;
}

static b32 IsNum(char At)
{
    return (At >= '0' && At <= '9') || (At == '-') || (At == '.');
}

static void EatAllWhiteSpace(tokenizer *Tokenizer)
{
    while (IsWhiteSpace(*Tokenizer->At))
    {
        ++Tokenizer->At;
    }
}

static token 
GetToken(tokenizer *Tokenizer,memory_partition* Partition)
{
    token Result;
    EatAllWhiteSpace(Tokenizer);
    
    string* TokenString;
    token_type Type;
    u32 TokenLength = 0;
    char *Start = Tokenizer->At;
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
                HasAdvanced = true;
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
                //HasAdvanced = true;
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
static token 
GetCSVToken(tokenizer *Tokenizer,memory_partition* Partition)
{
    token Result;
    //EatAllWhiteSpace(Tokenizer);
    
    while(*Tokenizer->At == ' '  ||
          *Tokenizer->At == '\t')
    {
        ++Tokenizer->At;
    }
    string* TokenString;
    token_type Type;
    u32 TokenLength = 0;
    char *Start = Tokenizer->At;
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

static token
GetCFGToken(tokenizer *Tokenizer, memory_partition* Partition)
{
	token Result;
	EatAllWhiteSpace(Tokenizer);

	string* TokenString;
	token_type Type;
	u32 TokenLength = 0;
	char *Start = Tokenizer->At;
	b32 HasAdvanced = false;
	while (!IsWhiteSpace(*Tokenizer->At))
	{
		switch (*Tokenizer->At)
		{
			case '/': //Comments considered white space
			{
				if (*(Tokenizer->At + 1) == '/')
			
					Result.Type = Token_Comment;
					while (!IsNewLine(*Tokenizer->At++))continue;
					Result.Data = CreateStringFromToChar(Tokenizer->At, "\n", Partition);
					HasAdvanced = true;
					return Result;
				}
				else if (*(Tokenizer->At + 1) == '*')
				{
					Result.Type = Token_Comment;
					while (!((*Tokenizer->At == '*') && (*(Tokenizer->At + 1)) == '/')) 
					{
						++Tokenizer->At;
						continue;
					}
					Result.Data = CreateStringFromToChar(Tokenizer->At, "\n", Partition);
					HasAdvanced = true;
					return Result;
				}
			}
			case '(': {Result.Type = Token_OpenParen; }break;
			case ')': {Result.Type = Token_CloseParen; }break;
			case '{': {Result.Type = Token_OpenBrace; }break;
			case '}': {Result.Type = Token_CloseBrace; }break;
			case ':': {Result.Type = Token_Colon; }break;
			case ',': {Result.Type = Token_Comma; }break;
			case '_': {Result.Type = Token_Underscore; }break;
			case '-': 
			{
				if (*(Tokenizer->At + 1) == '-')
				{

				}
				Result.Type = Token_Underscore; 
			}break;
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
				if (*Tokenizer->At == '\r' || *Tokenizer->At == '\n')
				{
					//Go back one to catch the new line or return carriage on next 
					//iteration.
					//Tokenizer->At--;
				}
				//HasAdvanced = true;
				return Result;
			}break;
		}
		if (!HasAdvanced)++Tokenizer->At;


	}
	return Result;
}

struct csv_field
{
    string Text;
};

struct csv_line
{
    vector Fields;
};

struct csv_data
{
    vector Lines;
};

//NOTE(ray):Output will be a vector
static csv_data
ParseCSV(memory_partition Memory, char* TextString)
{
    csv_data Data;
    u32 MemSize = 500;
    Data.Lines = CreateVector(MemSize, sizeof(csv_line));
    
    b32 IsParsing = true;
    tokenizer Tokenizer = {0};
    Tokenizer.At = TextString;
    vector TokenVector = CreateVector(MemSize, sizeof(token));
    
    //token *Token;
    u32 LineNumber = 0;
    csv_line* CurrentLine = PushAndCastEmptyVectorElement(csv_line,&Data.Lines);
    CurrentLine->Fields = CreateVector(10,sizeof(csv_field));
    token PrevToken;
    while(IsParsing)
    {
        token Token = GetCSVToken(&Tokenizer, &Memory);
        PushVectorElement(&TokenVector, &Token);
        
        if(Token.Type == Token_Identifier)
        {
            csv_field Field;
            Field.Text = *Token.Data;
            PushVectorElement(&CurrentLine->Fields,&Field);
        }
        else if(Token.Type == Token_ReturnCarriage || Token.Type == Token_NewLine)
        {
            if(PrevToken.Type != Token_ReturnCarriage)
            {
                ++LineNumber;
                CurrentLine = PushAndCastEmptyVectorElement(csv_line,&Data.Lines);
                CurrentLine->Fields = CreateVector(10,sizeof(csv_field));
            }
        }
        
        if (Token.Type == Token_EndOfStream)
        {
            break;
        }
        PrevToken = Token;
        
    }
    
    FreeVectorMem(&TokenVector);
    return Data;
}

struct cfg_entry
{
	string Key;
	string Text;
};

struct cfg_data
{
	vector Entries;
};

static b32 IsDoubleDash(token A,token B)
{
	if (A.Type == Token_Dash && B.Type == Token_Dash)
	{
		return true;
	}
	return false;
}

//NOTE(ray):Output will be a vector
static cfg_data
ParseConfig(memory_partition Memory, char* TextString)
{
	cfg_data Data;
	u32 MemSize = 500;
	Data.Entries = CreateVector(MemSize, sizeof(csv_line));

	b32 IsParsing = true;
	tokenizer Tokenizer = { 0 };
	Tokenizer.At = TextString;

	u32 LineNumber = 0;
	token PrevToken;
	while (IsParsing)
	{
		token Token = GetRawToken(&Tokenizer, &Memory);
		// Ignore single line type comments.
		if (Token.Type == Token_ForwardSlash && PrevToken.Type == Token_ForwardSlash)
		{
			//Is a comment line
			while (Token.Type != Token_NewLine || Token.Type != Token_ReturnCarriage)
			{
				Token = GetRawToken(&Tokenizer, &Memory);
			}
		}

		// Ignore block style comments.
		if (Token.Type != Token_Asterisk && PrevToken.Type == Token_Asterisk)
		{
			while (Token.Type != Token_ForwardSlash && PrevToken.Type == Token_Asterisk)
			{
				Token = GetRawToken(&Tokenizer, &Memory);
			}
		}
		// -- dash is the start of a config block
		if (!IsDoubleDash(Token,PrevToken))
		{
			Token = GetRawToken(&Tokenizer, &Memory)
			if(Token.Type == Token_Identifier && Compare(*Token.Data,"task"))
			{
				cfg_entry Entry;
				while (!IsDoubleDash(Token, PrevToken) || Token.Type != Token_EndOfStream)
				{
					Token = GetRawToken(&Tokenizer, &Memory);
					if (Token.Type == Token_Colon)
					{
						//next identifier is a data.
						//prev identifier is the key.

					}
					if (Token.Type == Token_Identifier)
					{
						Entry.Key = Token.Data;
					}

				}
			}
			
		}

		if (Token.Type == Token_EndOfStream)
		{
			break;
		}
		PrevToken = Token;

	}

	FreeVectorMem(&TokenVector);
	return Data;
}


#define API_TOKENIZER_H
#endif