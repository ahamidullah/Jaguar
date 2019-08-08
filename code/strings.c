s32 format_string(char *buffer, const char *format, va_list arguments) {
	return stbsp_vsprintf(buffer, format, arguments);
}

const char *find_first_occurrence_of_character(const char *s, char c) {
	while (*s && *s != c)
		s++;
	if (*s == c)
		return s;
	return NULL;
}

const char *find_last_occurrence_of_character(const char *string, char character) {
	const char *occurrence = NULL;
	while (*string) {
		if (*string == character) {
			occurrence = string;
		}
		string++;
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

size_t string_length(const char *s) {
	size_t count = 0;
	while (*s++)
		++count;
	return count;
}

void copy_string_range(char *destination, const char *source, size_t count) {
	for (s32 i = 0; i < count && source[i]; i++) {
		destination[i] = source[i];
	}
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

s8 compare_strings(const char *a, const char *b) {
	s32 i = 0;

	for (; a[i] != '\0'; ++i) {
		if (b[i] == '\0')
			return 1;
		if (a[i] > b[i])
			return 1;
		if (a[i] < b[i])
			return -1;
	}

	if (b[i] == '\0') {
		return 0;
	}
	return -1;
}

u8 strings_subset_test(const char **set_a, s32 set_a_string_count, const char **set_b, s32 set_b_string_count) {
	for (s32 i = 0; i < set_a_string_count; i++) {
		u8 found = 0;
		for (s32 j = 0; j < set_b_string_count; j++) {
			if (compare_strings(set_a[i], set_b[j]) == 0) {
				found = 1;
				break;
			}
		}
		if (!found) {
			return 0;
		}
	}
	return 1;
}
