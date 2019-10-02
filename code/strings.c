size_t c_string_length(const char *c_string) {
	size_t length = 0;
	while (*c_string++) {
		length++;
	}
	return length;
}

String S(const char *c_string) {
	size_t length = c_string_length(c_string);
	return (String){
		.data = (char *)c_string,
		.length = length,
		.capacity = length,
		.is_constant = 1,
	};
}

String create_string(u32 capacity, Memory_Arena *arena) {
	// We still null-terminate our strings to ensure compatibility with libraries, debuggers, etc.
	return (String){
		.data = allocate_array(arena, char, capacity + 1),
		.length = 0,
		.capacity = capacity,
		.is_constant = 0,
	};
}

s32 format_string(char *buffer, const char *format, va_list arguments) {
	return stbsp_vsprintf(buffer, format, arguments);
}

// @TODO: Use String.
const char *find_first_occurrence_of_character(const char *string, char character) {
	while (*string && *string != character) {
		string++;
	}
	if (*string != character) {
		return NULL;
	}
	return string;
}

u32 find_last_occurrence_of_character(String string, char character) {
	u32 occurrence = U32_MAX;
	for (u32 i = 0; i < string.length; i++) {
		if (string.data[i] == character) {
			occurrence = i;
		}
	}
	return occurrence;
}

// "Naive" approach. We don't use this for anthing perf critical now.
const char *find_substring(const char *s, const char *substring) {
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

void copy_string(char *destination, const char *source) {
	while (*source) {
		*destination++ = *source++;
	}
	*destination = '\0';
}

void reverse_string(char *start, char *end) {
	while (start < end) {
		char tmp = *start;
		*start++ = *end;
		*end-- = tmp;
	}
}

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

void append_string(String *destination, String source) {
	Assert(!destination->is_constant);
	Assert(destination->length + source.length <= destination->capacity);
	Copy_Memory(destination->data + destination->length, source.data, source.length);
	destination->length += source.length;
	destination->data[destination->length] = '\0';
}

void append_string_range(String *destination, String source, u32 source_start_index, u32 count) {
	Assert(!destination->is_constant);
	Assert(destination->length + count <= destination->capacity);
	for (u32 i = source_start_index; i < (source_start_index + count) && i < source.length; i++) {
		destination->data[destination->length++] = source.data[i];
	}
	destination->data[destination->length] = '\0';
}

String join_strings(String a, String b, Memory_Arena *arena) {
	debug_print("%s %s %u %u\n", a.data, b.data, a.length, b.length);
	size_t result_length = a.length + b.length + 1;
	String result = {
		.data = allocate_array(arena, char, result_length),
		.length = 0,
		.capacity = result_length,
	};
	append_string(&result, a);
	append_string(&result, b);
	result.data[result_length - 1] = '\0';
	return result;
}
