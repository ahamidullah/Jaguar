#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

u32 C_String_Length(const char *c_string) {
	u32 length = 0;
	while (*c_string++) {
		length++;
	}
	return length;
}

String S(const char *c_string) {
	u32 length = C_String_Length(c_string);
	return (String){
		.data = (char *)c_string,
		.length = length,
		.capacity = length,
		.is_constant = 1,
	};
}

String Create_String(u32 capacity) {
	// We still null-terminate our strings to ensure compatibility with libraries, debuggers, etc.
	return (String){
		.data = (char *)malloc(capacity + 1),
		.length = 0,
		.capacity = capacity,
		.is_constant = 0,
	};
}

s32 Format_String(char *buffer, const char *format, va_list arguments) {
	return stbsp_vsprintf(buffer, format, arguments);
}

s64 Find_First_Occurrence_Of_Character(String string, char character) {
	for (u32 i = 0; i < string.length; i++) {
		if (string.data[i] == character) {
			return i;
		}
	}
	return -1;
}

s64 Find_Last_Occurrence_Of_Character(String string, char character) {
	s32 occurrence = -1;
	for (u32 i = 0; i < string.length; i++) {
		if (string.data[i] == character) {
			occurrence = i;
		}
	}
	return occurrence;
}

// "Naive" approach. We don't use this for anthing perf critical now.
const char *Find_Substring(const char *s, const char *substring) {
	while (*s) {
		const char *subs = substring;
		if (*s != *subs) {
			++s;
			continue;
		}

		const char *sstart = s;
		while (*subs && *sstart && *sstart == *subs) {
			sstart++; subs++;
		}
		if (*subs == '\0')
			return s;
		++s;
	}
	return NULL;
}

void Copy_String(String source, String destination) {
	Assert(destination.capacity >= source.length + 1);
	for (u32 i = 0; i < source.length; i++) {
		destination.data[i] = source.data[i];
	}
	destination.length = source.length;
	destination.data[destination.length] = '\0';
}

bool Strings_Equal(const char *a, const char *b) {
	while (*a && *b) {
		if (*a != *b) {
			return false;
		}
		*a++;
		*b++;
	}
	if (*a || *b) {
		return false;
	}
	return true;
}

#if 0
s8 Compare_Strings(const char *a, const char *b) {
	s32 i = 0;
	for (; a[i] != '\0'; ++i) {
		if (b[i] == '\0') {
			return 1;
		}
		if (a[i] > b[i]) {
			return 1;
		}
		if (a[i] < b[i]) {
			return -1;
		}
	}
	if (b[i] == '\0') {
		return 0;
	}
	return -1;
}
#endif

void Append_String(String *destination, String source) {
	Assert(!destination->is_constant);
	Assert(destination->length + source.length <= destination->capacity);
	Copy_Memory(source.data, destination->data + destination->length, source.length);
	destination->length += source.length;
	destination->data[destination->length] = '\0';
}

void Append_String_Range(String *destination, String source, u32 source_start_index, u32 count) {
	Assert(!destination->is_constant);
	Assert(destination->length + count <= destination->capacity);
	for (u32 i = source_start_index; i < (source_start_index + count) && i < source.length; i++) {
		destination->data[destination->length++] = source.data[i];
	}
	destination->data[destination->length] = '\0';
}

String Join_Strings(String a, String b) {
	u32 result_length = a.length + b.length + 1;
	String result = {
		.data = (char *)malloc(result_length),
		.length = 0,
		.capacity = result_length,
	};
	Append_String(&result, a);
	Append_String(&result, b);
	result.data[result_length - 1] = '\0';
	return result;
}
