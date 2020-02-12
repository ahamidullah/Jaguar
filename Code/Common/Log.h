#pragma once

constexpr s32 PLATFORM_STANDARD_OUT = 1;
constexpr s32 PLATFORM_STANDARD_IN = 0;
constexpr s32 PLATFORM_STANDARD_ERROR = 2;

enum LogType {
	INFO_LOG,
	ERROR_LOG,
	ABORT_LOG,
};

#define PrintF32(F) PrintF32Actual(#F, (F))
#define PrintV3(V) PrintV3Actual(#V, (V))
#define PrintM4(M) PrintM4Actual(#M, (M))

void ConsolePrintVarargs(const char *format, va_list argument_list);
void ConsolePrint(const char *format, ...);
void LogPrint(LogType logType, const char *format, ...);

