#include "Log.h"

void ConsolePrintVarargs(const char *format, va_list argumentList) {
	char buffer[4096];
	s32 bytesWritten = FormatString(buffer, format, argumentList);
	PlatformWriteToFile(PLATFORM_STANDARD_OUT, bytesWritten, buffer);
}

void ConsolePrint(const char *format, ...) {
	char buffer[4096];
	va_list argumentList;
	va_start(argumentList, format);
	s32 bytesWritten = FormatString(buffer, format, argumentList);
	PlatformWriteToFile(PLATFORM_STANDARD_OUT, bytesWritten, buffer);
	va_end(argumentList);
}

// @TODO: Handle logType.
void LogPrint(LogType logType, const char *format, ...) {
	va_list argumentList;
	va_start(argumentList, format);
	// @TODO: Print message to a log file as well.
	if (debug) {
		ConsolePrintVarargs(format, argumentList);
	}
	va_end(argumentList);
}

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
