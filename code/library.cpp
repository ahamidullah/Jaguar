// @TODO: Handle log types.
void log_print_actual(Log_Type, const char *file_name, int line, const char *function_name, const char *format, ...) {
	va_list arguments;
	va_start(arguments, format);
	debug_print("%s: %s: %d: ", file_name, function_name, line);
	debug_print(format, arguments);
	debug_print("\n");
	va_end(arguments);
}

void _abort_actual(const char *file_name, s32 line, const char *function_name, const char *format, ...) {
	va_list arguments;
	va_start(arguments, format);
	debug_print("###########################################################################\n");
	debug_print("[PROGRAM ABORT]\n");
	debug_print("%s: %s: %d:\n", file_name, function_name, line);
	debug_print(format, arguments);
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
	auto file_handle = open_file(path, O_RDONLY); // @TODO: Platform generic flags.
	if (file_handle == FILE_HANDLE_ERROR) {
		return String_Result{NULL, 0};
	}
	auto file_length = get_file_length(file_handle);
	auto string_buffer = allocate_array(arena, char, (file_length+1));
	auto read_result = read_file(file_handle, file_length, string_buffer);
	if (!read_result) {
		return String_Result{NULL, 0};
	}

	return String_Result{string_buffer, file_length};
/*
	auto file_handle = open_file(path, O_RDONLY); // @TODO: Platform generic flags.
	if (file_handle == FILE_HANDLE_ERROR) {
		return STRING_ERROR;
	}
	auto file_length = seek_file(file_handle, 0, FILE_SEEK_END);
	if (file_length == FILE_OFFSET_ERROR) {
		return STRING_ERROR;
	}
	seek_file(fh, 0, FILE_SEEK_START);

	auto string_buffer = allocate_array(arena, char, (file_length+1));
	File_Offset total_bytes_read = 0, current_bytes_read = 0;
	char *position = string_buffer;
	// Read may return less bytes than requested, so we have to loop.
	do {
		current_bytes_read = read_file(file_handle, file_length - total_bytes_read, position);
		total_bytes_read += current_bytes_read;
		position += current_bytes_read;
	} while (total_bytes_read < file_length && current_bytes_read != 0);
	if (total_bytes_read != file_length) {
		return STRING_ERROR;
	}
	string_buffer[file_length] = '\0';
	close_file(file_handle);

	return {string_buffer, file_length};
*/
}

template <typename T>
T &Static_Array<T>::operator[](s32 index) {
	assert(index < capacity);
	return data[index];
}

const char *first_occurrence_of(const char *s, char c) {
	while (*s && *s != c)
		s++;
	if (*s == c)
		return s;
	return NULL;
}

// "Naive" approach. We don't use this for anthing perf critical now.
const char *first_occurrence_of(const char *s, const char *substring) {
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
	const char *s = (char *)source;
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

size_t push_integer(s32 i, char *buf) {
	size_t nbytes_writ = 0;
	if (i < 0) {
		buf[nbytes_writ++] = '-';
		i = -i;
	}
	char *start = buf + nbytes_writ;
	do {
		char ascii = (i % 10) + 48;
		buf[nbytes_writ++] = ascii;
		i /= 10;
	} while (i > 0);
	char *end = buf + nbytes_writ - 1;
	strrev(start, end);
	return nbytes_writ;
}

// @TODO: Handle floating point!!!!!
size_t format_string(const char *fmt, va_list arg_list, char *buf) {
	assert(fmt);
	size_t nbytes_writ = 0;
	for (const char *at = fmt; *at; ++at) {
		if (*at == '%') {
			++at;
			switch (*at) {
			case 's': {
				char *s = va_arg(arg_list, char *);
				assert(s);
				while (*s) buf[nbytes_writ++] = *s++;
			} break;
			case 'd': {
				int i = va_arg(arg_list, int);
				nbytes_writ += push_integer(i, buf + nbytes_writ);
			} break;
			case 'u': {
				int i = va_arg(arg_list, unsigned);
				nbytes_writ += push_integer(i, buf + nbytes_writ);
			} break;
			case 'c': {
				buf[nbytes_writ++] = va_arg(arg_list, int);
			} break;
			case 'l': {
				long l = va_arg(arg_list, long);
				nbytes_writ += push_integer(l, buf + nbytes_writ);
			} break;
			default: {

			} break;
			}
		} else {
			buf[nbytes_writ++] = *at;
		}
	}
	buf[nbytes_writ++] = '\0'; // NULL terminator.
	return nbytes_writ;
}

u8 strings_subset_test(const char **set_a, s32 num_set_a_strings, const char **set_b, s32 num_set_b_strings) {
	for (s32 i = 0; i < num_set_a_strings; i++) {
		u8 found = false;
		for (s32 j = 0; j < num_set_b_strings; j++) {
			if (compare_strings(set_a[i], set_b[j]) == 0) {
				found = true;
				break;
			}
		}
		if (!found) {
			return false;
		}
	}

	return true;
}

const char *get_directory(const char *path, Memory_Arena *arena) {
	auto slash = first_occurrence_of(path, '/');
	if (slash == NULL) {
		return path;
		//char *directory = allocate_array(string_length(path) + 1);
		//return copy_string(directory, path);
	}

	return slash + 1;
	//auto directory_length = (slash - path) + 1; // Including the '/'.
	//auto directory = allocate_array(arena, char, directory_length + 1);//(char *)get_temporary_storage(directory_len + 1);
	//copy_string(directory, path, directory_len);
	//directory[directory_len] = '\0';

	//return directory;
}

#if 0
#define TIMED_BLOCK(name) Block_Timer __block_timer__##__LINE__(#name)
#define MAX_TIMER_NAME_LEN 256

struct Block_Timer {
	Block_Timer(const char *n)
	{
		size_t name_len = _strlen(n);
		assert(name_len < MAX_TIMER_NAME_LEN);
		string_copy(name, n);
		start = platform_get_time();
	}
	~Block_Timer()
	{
		Time_Spec end = platform_get_time();
		unsigned ns_res=1, us_res=1000, ms_res=1000000;
		long ns = platform_time_diff(start, end, ns_res);
		long ms = platform_time_diff(start, end, ms_res);
		long us = platform_time_diff(start, end, us_res);
		debug_print("%s - %dns %dus %dms\n", name, ns, us, ms);
	}
	Time_Spec start;
	char name[MAX_TIMER_NAME_LEN];
};

/*
bool
get_base_name(const char *path, char *name_buf)
{
	int begin = 0;
	while(path[begin] && path[begin] != '/')
		++begin;
	if (!path[begin])
		return false;
	int end = ++begin;
	while(path[end] && path[end] != '.')
		++end;
	strncpy(name_buf, path+begin, end-begin);
	name_buf[end-begin] = '\0';
	return true;
}
*/

Color
random_color()
{
	float r = rand() / (float)RAND_MAX;
	float g = rand() / (float)RAND_MAX;
	float b = rand() / (float)RAND_MAX;
	return Color{r, g, b};
}

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

template<typename T>
void
array_remove(Array<T> *a, size_t index)
{
	assert(index >= 0 && index < a->size);

	for (u32 i = index + 1; i < a->size; ++i) {
		a->data[i - 1] = a->data[i];
	}

	a->size -= 1;
}
#endif
