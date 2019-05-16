/*
String
read_entire_file(const char *path, Memory_Arena *ma)
{
	File_Handle fh = platform_open_file(path, O_RDONLY); // @TODO: Platform generic flags.
	if (fh == FILE_HANDLE_ERROR)
		return STRING_ERROR;
	File_Offset len = platform_seek_file(fh, 0, FILE_SEEK_END);
	if (len == FILE_OFFSET_ERROR)
		return STRING_ERROR;
	platform_seek_file(fh, 0, FILE_SEEK_START);
	char *buf = mem_alloc_array(char, (len+1), ma);
	// Read may return less bytes than requested, so we have to loop.
	File_Offset tot_read = 0, cur_read = 0;
	char *pos = buf;
	do {
		cur_read = platform_read_file(fh, len - tot_read, pos);
		tot_read += cur_read;
		pos += cur_read;
	} while (tot_read < len && cur_read != 0);
	if (tot_read != len)
		return STRING_ERROR;
	buf[len] = '\0';
	platform_close_file(fh);
	return String{buf, len};
}
*/

#define ARRAY_COUNT(x) (sizeof(x)/sizeof(x[0]))

const char *strchr(const char *s, char c) {
	while (*s && *s != c)
		s++;
	if (*s == c)
		return s;
	return NULL;
}

// "Naive" approach. We don't use this for anthing perf critical now.
const char *_strstr(const char *s, const char *substring) {
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

size_t _strlen(const char *s) {
	size_t count = 0;
	while (*s++)
		++count;
	return count;
}

void string_copy(char *dest, const char *src) {
	while (*src) {
		*dest++ = *src++;
	}
	*dest = '\0';
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

// @TODO: Handle log types.
void log_print_actual(Log_Type, const char *file, int line, const char *func, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	debug_print("%s: %s: line %d: ", file, func, line);
	debug_print(fmt, args);
	debug_print("\n");
	va_end(args);
}

void _abort_actual(const char *file, s32 line, const char *func, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	debug_print("!!!!!!!!!!!!!!!!!!!!!!!!!!PROGRAM ABORT!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n*\n*\n*\n");
	debug_print("%s: %s: line %d: ", file, func, line);
	debug_print(fmt, args);
	debug_print("\n*\n*\n*\n");
	va_end(args);
#ifdef DEBUG
	assert(0);
#else
	_exit(1);
#endif
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
