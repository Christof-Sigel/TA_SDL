#include <stdlib.h>

internal char * GetStringValue(TDFElement * Element, const char * Name)
{
    TDFNameValue * NameValue = Element->Value;
    while(NameValue)
    {
	if(CaseInsensitiveMatch(Name, NameValue->Name))
	    return NameValue->Value;
	NameValue = NameValue->Next;
    }
    return 0;
}

internal int GetIntValue(TDFElement * Element, const char * Name)
{
    char * Value=GetStringValue(Element,Name);
    if(Value)
	return atoi(Value);
    return -1;
}

internal float GetFloat(TDFElement * Element, const char * Name)
{
    char * Value= GetStringValue(Element,Name);
    if(Value)
	return (float)atof(Value);
    return -1;
}

internal TDFElement * GetSubElement(TDFElement * Root, const char * Name)
{
    TDFElement * element = Root->Child;
    while(element)
    {
	if(CaseInsensitiveMatch(Name, element->Name))
	    return element;
	element=element->Next;
    }
    return 0;
}

internal TDFElement * GetNthElement(TDFElement * Start, int N)
{
    if(N ==0 || !Start)
	return Start;
    return GetNthElement(Start->Next, N-1);
}

internal int CountElements(TDFElement * First)
{
    int count = 0;
    while(First)
    {
	First = First->Next;
	count++;
    }
    return count;
}

internal TDFNameValue * LoadTDFNameValueFromBuffer(char ** InBuffer, char * End, MemoryArena * Arena)
{
    char * Buffer = *InBuffer;
    char * Start=Buffer;
    while(*Buffer != '=' && *Buffer != ' ' && *Buffer != '\t' && *Buffer != '\r' && *Buffer!='\n' && Buffer <=End) {  Buffer++;  }
    if(Buffer == End){	return 0; }
    char sep = *Buffer;
    *Buffer =0;
    TDFNameValue * Result = PushStruct(Arena, TDFNameValue);
    *Result={};
    strncpy(Result->Name, Start, TDF_MAX_STRING_LENGTH);
    Buffer++;
    if(sep != '=')
    {
	while((*Buffer == '=' || *Buffer != ' ' || *Buffer != '\t' || *Buffer != '\r' || *Buffer!='\n')
	      && Buffer <=End) {  Buffer++;  }
	if(Buffer >= End){	return 0; }
    }
    Start = Buffer;
    while(*Buffer != ';' && Buffer <=End) {  Buffer++;  }
    if(Buffer >= End){	return 0; }
    *Buffer =0;
    strncpy(Result->Value, Start, TDF_MAX_STRING_LENGTH);
    Buffer++;

    *InBuffer = Buffer;
    return Result;
}


internal TDFElement * LoadTDFElementFromBuffer(char ** InBuffer, char * End, MemoryArena * Arena)
{
    char * Buffer = *InBuffer;
    while(*Buffer != '[' && Buffer <=End) {  Buffer++;  }
    if(Buffer == End){	return 0; }
    char * Start = Buffer+1;
    while(*Buffer != ']' && Buffer <=End) {  Buffer++;  }
    if(Buffer == End){	return 0; }
    *Buffer =0;

    TDFElement * Result = PushStruct(Arena, TDFElement);
    *Result={};
    strncpy(Result->Name, Start, TDF_MAX_STRING_LENGTH);
    while(*Buffer != '{' && Buffer <=End) {  Buffer++;  }
    if(Buffer == End){	return 0; }
    Buffer++;

    while(Buffer<=End)
    {
	while((*Buffer == ' ' || *Buffer == '\t' || *Buffer =='\r' || *Buffer == '\n' )&& Buffer <=End) {  Buffer++;  }
	if(Buffer == End){	return 0; }
	if(*Buffer =='}')
	{
	    Buffer++;
	    *InBuffer = Buffer;
	    return Result;
	}
	else if(*Buffer == '[')
	{
	    if(Result->Child)
	    {
		TDFElement * Child = Result->Child;
		while(Child->Next)
		{
		    Child=Child->Next;
		}
		Child->Next=LoadTDFElementFromBuffer(&Buffer,End, Arena);
	    }
	    else
	    {
		Result->Child = LoadTDFElementFromBuffer(&Buffer, End, Arena);
	    }
	}
	else
	{
	    if(Result->Value)
	    {
		TDFNameValue * Value = Result->Value;
		while(Value->Next)
		{
		    Value=Value->Next;
		}
		Value->Next = LoadTDFNameValueFromBuffer(&Buffer, End, Arena);
	    }
	    else
	    {
		Result->Value=LoadTDFNameValueFromBuffer(&Buffer, End, Arena);
	    }
	}

    }
    *InBuffer = Buffer;
    return 0;
}

internal TDFElement * LoadTDFElementsFromBuffer(char ** InBuffer, char * End, MemoryArena * Arena)
{
    TDFElement * First = LoadTDFElementFromBuffer(InBuffer, End, Arena);
    TDFElement * Last = First;
    while(*InBuffer <End)
    {
	Last->Next = LoadTDFElementFromBuffer(InBuffer, End, Arena);
	if(Last->Next)
	{
	    Last = Last->Next;
	}
    }
    return First;
}
