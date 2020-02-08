#include "Strings.h"
#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

char &String::operator[](size_t I) {
	Assert(I < Data.Count);
	return Data[I];
}

char &String::operator[](size_t I) const {
	Assert(I < Data.Count);
	return Data[I];
}

String::operator bool() {
	return true; // @TODO
}

bool operator==(const String &A, const String &B) {
	return A.Data == B.Data;
}

String CreateString(size_t Length, size_t Capacity) {
	String Result = {
		.Data = CreateArray<char>(Length + 1, Capacity + 1),
	};
	Result.Data[Length] = '\0';
	return Result;
}

String CreateString(size_t Length) {
	String Result = {
		.Data = CreateArray<char>(Length + 1),
	};
	Result.Data[Length] = '\0';
	return Result;
}

void Append(String *Destination, const String &Source) {
	size_t WriteIndex = Length(Destination);
	Resize(&Destination->Data, Length(Destination) + Length(Source) + 1);
	Copy_Memory(&Source.Data[0], &Destination->Data[WriteIndex], Length(Source) + 1);
}

size_t Length(const char *S) {
	return strlen(S); // @TODO
}

void Append(String *Destination, const char *Source) {
	size_t SourceLength = Length(Source);
	size_t WriteIndex = Length(Destination);
	Resize(&Destination->Data, Length(Destination) + SourceLength + 1);
	Copy_Memory(Source, &Destination->Data[WriteIndex], SourceLength + 1);
}

void Append(String *Destination, String Source, u32 RangeStartIndex, u32 RangeLength) {
	Assert(Source.Data.Count > RangeStartIndex);
	Assert(Source.Data.Count >= RangeStartIndex + RangeLength);
	size_t WriteIndex = Length(Destination);
	Resize(&Destination->Data, Length(Destination) + RangeLength + 1);
	Copy_Memory(&Source.Data[RangeStartIndex], &Destination->Data[WriteIndex], RangeLength + 1);

	///SetSize(&Destination->Data, Length(Destination->Data) - 1);
	///Append(&Destination->Data, &Source.Data[RangeStartIndex], RangeLength);
	///Append(&Destinaton->Data, '\0');
}

size_t Length(const String *S) {
	if (Length(S->Data) == 0) {
		return 0;
	}
	return Length(S->Data) - 1;
}

size_t Length(const String &S) {
	if (Length(S.Data) == 0) {
		return 0;
	}
	return Length(S.Data) - 1; // To account for the NULL terminator we insert at the end.
}

String FormatString(const char *format, ...) {
	va_list argument_list;
	va_start(argument_list, format);
	String result = CreateString(100); // @TODO
	stbsp_vsprintf(&result[0], format, argument_list);
	va_end(argument_list);
	return result;
}

s32 FormatString(char *Buffer, const char *Format, va_list Arguments) {
	return stbsp_vsprintf(Buffer, Format, Arguments);
}

s64 FindFirstOccurrenceOfCharacter(const String &S, char C) {
	for (size_t I = 0; I < Length(S); I++) {
		if (S.Data[I] == C) {
			return I;
		}
	}
	return -1;
}

s64 FindLastOccurrenceOfCharacter(const String &S, char C) {
	s64 Occurrence = -1;
	for (size_t I = 0; I < Length(S); I++) {
		if (S.Data[I] == C) {
			Occurrence = I;
		}
	}
	return Occurrence;
}

char *begin(String &S) {
	return &S.Data[0];
}

char *end(String &S) {
	return &S.Data[S.Data.Count];
}

char *begin(String *S) {
	return &S->Data[0];
}

char *end(String *S) {
	return &S->Data[S->Data.Count];
}

