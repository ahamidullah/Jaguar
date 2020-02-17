void ConsolePrint(const char *format, va_list arguments)
{
	char buffer[4096];
	auto bytesWritten = FormatString(buffer, format, arguments);
	WriteFile(STANDARD_OUT, bytesWritten, buffer);
}

void ConsolePrint(const char *format, ...)
{
	char buffer[4096];
	va_list arguments;
	va_start(arguments, format);
	s32 bytesWritten = FormatString(buffer, format, arguments);
	WriteFile(STANDARD_OUT, bytesWritten, buffer);
	va_end(arguments);
}

void LogPrint(LogType logType, const char *format, va_list arguments)
{
	// @TODO: Print message to a log file as well.
	if (debug)
	{
		ConsolePrint(format, arguments);
	}
}

// @TODO: Handle logType.
void LogPrint(LogType logType, const char *format, ...)
{
	va_list arguments;
	va_start(arguments, format);
	LogPrint(logType, format, arguments);
	va_end(arguments);
}

/*
void PrintM4Actual(const char *name, M4 m) {
	ConsolePrint("%s:\n", name);
	for (s32 i = 0; i < 4; i++) {
		ConsolePrint("%f %f %f %f\n", m[i][0], m[i][1], m[i][2], m[i][3]);
	}
}

void PrintV3Actual(const char *name, V3 v) {
	ConsolePrint("%s: %f %f %f\n", name, v.X, v.Y, v.Z);
}

void PrintF32Actual(const char *name, f32 number) {
	ConsolePrint("%s: %f\n", name, number);
}
*/
