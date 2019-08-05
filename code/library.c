s32 format_string(char *buffer, const char *format, va_list arguments) {
	return stbsp_vsprintf(buffer, format, arguments);
}

void debug_print_va_list(const char *format, va_list argument_list) {
	char buffer[4096];
	s32 bytes_written = format_string(buffer, format, argument_list);
	write_file(STDOUT, bytes_written, buffer);
}

void debug_print(const char *format, ...) {
	char buffer[4096];
	va_list argument_list;
	va_start(argument_list, format);
	s32 bytes_written = format_string(buffer, format, argument_list);
	write_file(STDOUT, bytes_written, buffer);
	va_end(argument_list);
}

// @TODO: Handle log types.
void log_print_actual(Log_Type log_type, const char *file_name, int line, const char *function_name, const char *format, ...) {
	va_list arguments;
	va_start(arguments, format);
	debug_print("%s: %s: %d: ", file_name, function_name, line);
	debug_print_va_list(format, arguments);
	debug_print("\n");
	va_end(arguments);
}

void _abort_actual(const char *file_name, s32 line, const char *function_name, const char *format, ...) {
	va_list arguments;
	va_start(arguments, format);
	debug_print("###########################################################################\n\n");
	debug_print("[PROGRAM ABORT]\n");
	debug_print("%s: %s: %d:\n", file_name, function_name, line);
	debug_print_va_list(format, arguments);
	debug_print("\n\n");
	print_stacktrace();
	debug_print("\n###########################################################################\n");
	va_end(arguments);

#ifdef DEBUG
	assert(0);
#else
	_exit(1);
#endif
}

u32 u32_max(u32 a, u32 b) {
	return a > b ? a : b;
}

u32 u32_min(u32 a, u32 b) {
	return a < b ? a : b;
}

String_Result read_entire_file(const char *path, Memory_Arena *arena) {
	File_Handle file_handle = open_file(path, O_RDONLY); // @TODO: Platform generic flags.
	if (file_handle == FILE_HANDLE_ERROR) {
		return (String_Result){NULL, 0};
	}
	File_Offset file_length = get_file_length(file_handle);
	char *string_buffer = allocate_array(arena, char, (file_length+1));
	u8 read_result = read_file(file_handle, file_length, string_buffer);
	if (!read_result) {
		return (String_Result){NULL, 0};
	}
	return (String_Result){string_buffer, file_length};
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

void strrev(char *start, char *end) {
	while (start < end) {
		char tmp = *start;
		*start++ = *end;
		*end-- = tmp;
	}
}

// Only legal if source and destination are in the same array.
void _memmove(void *destination, void *source, size_t len) {
	char *s = (char *)source;
	char *d = (char *)destination;
	if (s < d) {
		for (s += len, d += len; len; --len) {
			*--d = *--s;
		}
	} else {
		while (len--) {
			*d++ = *s++;
		}
	}
}

void _memcpy(void *destination, const void *source, size_t count) {
	const char *s = (const char *)source;
	char *d = (char *)destination;
	for (size_t i = 0; i < count; ++i) {
		d[i] = s[i];
	}
}

void _memset(void *destination, int set_to, size_t count) {
	char *d = (char *)destination;
	for (size_t i = 0; i < count; ++i) {
		d[i] = set_to;
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

u8 strings_subset_test(const char **set_a, s32 num_set_a_strings, const char **set_b, s32 num_set_b_strings) {
	for (s32 i = 0; i < num_set_a_strings; i++) {
		u8 found = 0;
		for (s32 j = 0; j < num_set_b_strings; j++) {
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

const char *get_directory(const char *path, Memory_Arena *arena) {
	const char *slash = find_last_occurrence_of_character(path, '/');
	if (slash == NULL) {
		return "";
	}
	size_t directory_length = slash - path;
	char *directory = allocate_array(arena, char, directory_length + 1);
	copy_string_range(directory, path, directory_length);
	directory[directory_length] = '\0';
	return directory;
}

char *join_paths(const char *path_1, const char *path_2, Memory_Arena *arena) {
	size_t path_1_length = string_length(path_1);
	size_t path_2_length = string_length(path_2);
	char *result = allocate_array(arena, char, path_1_length + path_2_length + 2);
	copy_string(result, path_1);
	result[path_1_length] = '/';
	copy_string(result + path_1_length + 1, path_2);
	result[path_1_length + path_2_length + 1] = '\0';
	return result;
}

/*
#define TIMED_BLOCK(name) Block_Timer __block_timer__##__LINE__(#name)
#define MAX_TIMER_NAME_LEN 256

struct Block_Timer {
	Block_Timer(const char *timer_name) {
		//size_t name_len = _strlen(n);
		//assert(name_len < MAX_TIMER_NAME_LEN);
		copy_string(name, timer_name);
		start = get_current_platform_time();
	}
	~Block_Timer() {
		Platform_Time end = get_current_platform_time();
		unsigned ns_res=1, us_res=1000, ms_res=1000000;
		long ns = platform_time_difference(start, end, ns_res);
		long ms = platform_time_difference(start, end, ms_res);
		long us = platform_time_difference(start, end, us_res);
		debug_print("%s - %dms %dus %dns\n", name, ms, us, ns);
	}

	Platform_Time start;
	char name[MAX_TIMER_NAME_LEN];
};
*/

void print_m4_actual(const char *name, M4 matrix) {
	debug_print("%s:\n", name);
	for (s32 i = 0; i < 4; i++) {
		debug_print("%f %f %f %f\n", matrix.m[i][0], matrix.m[i][1], matrix.m[i][2], matrix.m[i][3]);
	}
}

V3 random_color() {
	float r = rand() / (float)RAND_MAX;
	float g = rand() / (float)RAND_MAX;
	float b = rand() / (float)RAND_MAX;
	return (V3){r, g, b};
}

#if 0

void
timer_set(u32 wait_time, Timer *t)
{
	t->wait_time  = wait_time;
	t->start_time = platform_get_time_ms();
}

bool
timer_check_one_shot(Timer *t)
{
	if (t->start_time == 0) {
		return true;
	}

	u32 current_time = platform_get_time_ms();

	if ((current_time - t->start_time) >= t->wait_time) {
		t->start_time = 0;
		return true;
	}

	return false;
}

bool
timer_check_repeating(Timer *t)
{
	u32 current_time = platform_get_time_ms();

	if ((current_time - t->start_time) >= t->wait_time) {
		t->start_time = current_time;
		return true;
	}

	return false;
}
#endif
