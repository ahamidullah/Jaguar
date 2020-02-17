#pragma once

enum struct LogType {
	INFO,
	ERROR,
	ABORT,
};

#define PrintF32(F) PrintF32Actual(#F, (F))
#define PrintV3(V) PrintV3Actual(#V, (V))
#define PrintM4(M) PrintM4Actual(#M, (M))

void ConsolePrintVarargs(const char *format, va_list argumentList);
void ConsolePrint(const char *format, ...);
void LogPrint(LogType logType, const char *format, ...);
