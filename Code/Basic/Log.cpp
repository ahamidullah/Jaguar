#include "Log.h"
#include "String.h"
#include "File.h"
#include "Memory.h"
#include "Process.h"

#if DEBUG_BUILD
	const auto LogFileDir = String{"Data/Log/"};

	auto globalLogFile = File{};
	auto globalLogFileOpen = false;
	auto logLevel = LogLevelInfo;
	auto logPool = PoolAllocator{};
#endif

void InitalizeLog()
{
	#if DEBUG_BUILD
		// We want to call this before memory has been initialized, so we have to use a stack-based allocator.
		auto buf = StaticArray<char, 512>{};
		auto a = NewStackAllocatorIn(sizeof(buf), buf.elements);
		auto sb = NewStringBuilderIn(0, NewStackAllocatorInterface(&a));

		AppendToStringBuilder(&sb, LogFileDir);
		FormatTimestamp(&sb, CurrentTime());
		auto dir = BuilderToString(sb);
		if (!CreateDirectory(dir))
		{
			LogPrint(LogLevelError, "Log", "Failed to create log directory %k.\n", dir);
			LogPrint(LogLevelInfo, "Log", "Initialized logging.\n");
			return;
		}

		AppendToStringBuilder(&sb, "/");
		AppendToStringBuilder(&sb, "Global.log");
		auto path = BuilderToString(sb);
		auto err = false;
		globalLogFile = OpenFile(path, OpenFileWriteOnly | OpenFileCreate, &err);
		if (err)
		{
			LogPrint(LogLevelError, "Log", "Failed to open global log file %k.\n", path);
			LogPrint(LogLevelInfo, "Log", "Initialized logging.\n");
			return;
		}

		Assert(sb.buffer.count < sizeof(buf));
		globalLogFileOpen = true;
		LogPrint(LogLevelInfo, "Log", "Initialized logging.\n");
	#endif
}

void ConsolePrintVarArgs(String fmt, va_list args)
{
	#if DEBUG_BUILD
		PushContextAllocator(NewPoolAllocatorInterface(&logPool));
		Defer(
		{
			PopContextAllocator();
			ClearPoolAllocator(&logPool);
		});
		auto sb = StringBuilder{};
		FormatStringVarArgs(&sb, fmt, args);
		WriteToConsole(BuilderToString(sb));
	#endif
}

void ConsolePrint(String fmt, ...)
{
	#if DEBUG_BUILD
		va_list args;
		va_start(args, fmt);
		ConsolePrintVarArgs(fmt, args);
		va_end(args);
	#endif
}

void ConsolePrint(const char *fmt, ...)
{
	#if DEBUG_BUILD
		va_list args;
		va_start(args, fmt);
		ConsolePrintVarArgs(fmt, args);
		va_end(args);
	#endif
}

String LogLevelToString(LogLevel l)
{
	switch (l)
	{
	case LogLevelVerbose:
	{
		return "Verbose";
	} break;
	case LogLevelInfo:
	{
		return "Info";
	} break;
	case LogLevelError:
	{
		return "Error";
	} break;
	case LogLevelAbort:
	{
		return "Abort";
	} break;
	default:
	{
		Abort("Unknown log level %d.", l);
	} break;
	}
}

void LogPrintVarArgs(String file, String func, s64 line, LogLevel l, String category, String fmt, va_list args)
{
	#if DEBUG_BUILD
		PushContextAllocator(NewPoolAllocatorInterface(&logPool));
		Defer(
		{
			PopContextAllocator();
			ClearPoolAllocator(&logPool);
		});
		auto sb = StringBuilder{};
		{
			FormatStringVarArgs(&sb, fmt, args);
			WriteToConsole(BuilderToString(sb));
		}
		ClearStringBuilder(&sb);
		if (globalLogFileOpen)
		{
			FormatString(&sb, "[%k]  ", LogLevelToString(l));
			FormatTime(&sb, "%d-%d-%d %d:%d:%d.%d  ", CurrentTime());
			FormatString(&sb, "%k:%d %k  |  ", file, line, func);
			FormatStringVarArgs(&sb, fmt, args);
			WriteToFile(globalLogFile, sb.length, sb.buffer.elements);
		}
	#endif
}

void LogPrintActual(String file, String func, s64 line, LogLevel l, String category, String fmt, ...)
{
	#if DEBUG_BUILD
		if (logLevel < l)
		{
			return;
		}
		va_list args;
		va_start(args, fmt);
		LogPrintVarArgs(file, func, line, l, category, fmt, args);
		va_end(args);
	#endif
}

void LogPrintActual(const char *file, const char *func, s64 line, LogLevel l, String category, const char *fmt, ...)
{
	#if DEBUG_BUILD
		if (logLevel < l)
		{
			return;
		}
		va_list args;
		va_start(args, fmt);
		LogPrintVarArgs(file, func, line, l, category, fmt, args);
		va_end(args);
	#endif
}

void SetLogLevel(LogLevel l)
{
	#if DEBUG_BUILD
		logLevel = l;
	#endif
}
