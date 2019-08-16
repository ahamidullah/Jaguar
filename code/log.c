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

void print_m4_actual(const char *name, M4 matrix) {
	debug_print("%s:\n", name);
	for (s32 i = 0; i < 4; i++) {
		debug_print("%f %f %f %f\n", matrix.m[i][0], matrix.m[i][1], matrix.m[i][2], matrix.m[i][3]);
	}
}

void print_v3_actual(const char *name, V3 vector) {
	debug_print("%s: %f %f %f\n", name, vector.x, vector.y, vector.z);
}

// @TODO: Move me.
V3 random_color() {
	float r = rand() / (float)RAND_MAX;
	float g = rand() / (float)RAND_MAX;
	float b = rand() / (float)RAND_MAX;
	return (V3){r, g, b};
}

