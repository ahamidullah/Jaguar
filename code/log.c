// @TODO: Rename debug_print to Console_Print.
// @TODO: Replace a lot of debug printing with log printing or switch it to development only.
// @TODO: Option to Log_Print without header info.
void Console_Printv(const char *format, va_list argument_list) {
	char buffer[4096];
	s32 bytes_written = format_string(buffer, format, argument_list);
	Platform_Write_To_File(STDOUT, bytes_written, buffer);
}

void Console_Print(const char *format, ...) {
	char buffer[4096];
	va_list argument_list;
	va_start(argument_list, format);
	s32 bytes_written = format_string(buffer, format, argument_list);
	Platform_Write_To_File(STDOUT, bytes_written, buffer);
	va_end(argument_list);
}

// @TODO: Handle log types.
// @TODO: Log to file.
void Log_Print(Log_Type log_type, const char *format, ...) {
	va_list arguments;
	va_start(arguments, format);
	// @TODO: Print message to a log file as well.
#if defined(DEBUG)
	Console_Printv(format, arguments);
#endif
	va_end(arguments);
}

void Abort(const char *format, ...) {
	// @TODO: Print message to a log file as well.
#if defined(DEBUG)
	va_list arguments;
	va_start(arguments, format);
	Console_Print("###########################################################################\n");
	Console_Print("[PROGRAM ABORT]\n");
	Console_Printv(format, arguments);
	Console_Print("\n");
	Platform_Print_Stacktrace();
	Console_Print("###########################################################################\n");
	va_end(arguments);
	Platform_Signal_Debug_Breakpoint();
#endif
	_exit(1);
}

void print_m4_actual(const char *name, M4 matrix) {
	Console_Print("%s:\n", name);
	for (s32 i = 0; i < 4; i++) {
		Console_Print("%f %f %f %f\n", matrix.m[i][0], matrix.m[i][1], matrix.m[i][2], matrix.m[i][3]);
	}
}

void print_v3_actual(const char *name, V3 vector) {
	Console_Print("%s: %f %f %f\n", name, vector.x, vector.y, vector.z);
}

void print_f32_actual(const char *name, f32 number) {
	Console_Print("%s: %f\n", name, number);
}
