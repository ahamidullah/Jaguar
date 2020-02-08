#pragma once

#include "Array.h"

size_t Length(const char *S);

// @TODO: Make this utf-8 by default?
struct String {
	Array<char> Data;
	String() = default;
	String(const Array<char> &A) : Data{A} {}
	String(const char *CString) : Data{CreateArray(Length(CString) + 1, CString)} {};
	char &operator[](size_t I);
	char &operator[](size_t I) const;
	operator bool();
};

String CreateString(size_t Length, size_t Capacity);
String CreateString(size_t Length);
size_t Length(const String &S);
size_t Length(const String *S);
void Append(String *Destination, const String &Source);
void Append(String *Destination, const char *Source);
void Append(String *Destination, String Source, u32 RangeStartIndex, u32 RangeLength);
String FormatString(const char *format, ...);
s64 FindFirstOccurrenceOfCharacter(const String &S, char C);
s64 FindLastOccurrenceOfCharacter(const String &S, char C);

template<typename... StringPack>
size_t Length(const String& S, const StringPack... Rest) {
	return Length(S) + Length(Rest...);
}

template<typename... StringPack>
size_t Length(const char *S, const StringPack... Rest) {
	return Length(S) + Length(Rest...);
}

template<typename... StringPack>
void ConcatenateAppend(size_t WriteIndex, String *Result, const String &Source) {
	Copy_Memory(&Source.Data[0], &Result->Data[WriteIndex], Length(Source));
}

template<typename... StringPack>
void ConcatenateAppend(size_t WriteIndex, String *Result, const char *Source) {
	Copy_Memory(Source, &Result->Data[WriteIndex], Length(Source));
}

template<typename... StringPack>
void ConcatenateAppend(size_t WriteIndex, String *Result, const String &First, StringPack... Rest) {
	Copy_Memory(&First.Data[0], &Result->Data[WriteIndex], Length(First));
	ConcatenateAppend(WriteIndex + Length(First), Result, Rest...);
}

template<typename... StringPack>
void ConcatenateAppend(size_t WriteIndex, String *Result, const char *First, StringPack... Rest) {
	Copy_Memory(First, &Result->Data, Length(First));
	ConcatenateAppend(WriteIndex + Length(First), Result, Rest...);
}

template<typename T, typename... StringPack>
String Concatenate(const T &First, const StringPack... Rest) {
	String Result = CreateString(Length(First, Rest...));
	ConcatenateAppend(0, &Result, First, Rest...);
	Result.Data[Length(Result)] = '\0';
	return Result;
}
