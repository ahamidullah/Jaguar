#include "Log.h"
#include "Basic/File.h"
#include "Basic/Proc.h"
#include "Basic/Thread.h"
#include "Basic/Atomic.h"
#include "Basic/Mem.h"

namespace log
{

struct Logger
{
	sync::Spinlock lock;
	Level level;
	fs::File file;
	arr::array<CrashHandler> crashHandlers;

	void Print(str::String file, str::String func, s64 line, LogLevel l, str::String cat, str::String fmt, ...);
};

Logger New(Level l, fs::File f)
{
	return
	{
		.level = l,
		.file = f,
		.crashHandlers = arr::New<CrashHandler>(mem::ContextAllocator(), 0),
	}
}

auto logger = New(Level::Info, fs::File{}); // @TODO

Level CurrentLevel()
{
	return logger.level;
}

void SetLevel(Level l)
{
	logger.level = l;
}

void ConsoleVarArgs(str::String fmt, va_list args)
{
	auto sb = str::Builder{};
	sb.FormatVarArgs(fmt, args);
	ConsoleWrite(sb.View(0, sb.Length()));
}

void Console(str::String fmt, ...)
{
	#if DebugBuild
		va_list args; // Clang complains if we use 'auto' style declartions with va_list for some reason...
		va_start(args, fmt);
		ConsolePrintVarArgs(fmt, args);
		va_end(args);
	#endif
}

void ConsolePrintCtx(str::String fmt, ...)
{
	#if DebugBuild
		va_list args; // Clang complains if we use 'auto' style declartions with va_list for some reason...
		va_start(args, fmt);
		ConsolePrintVarArgs(fmt, args);
		va_end(args);
	#endif
}

str::String LevelToString(Level l)
{
	switch (l)
	{
	case Level::Verbose:
	{
		return "Verbose";
	} break;
	case Level::Info:
	{
		return "Info";
	} break;
	case Level::Error:
	{
		return "Error";
	} break;
	case Level::Fatal:
	{
		return "Fatal";
	} break;
	default:
	{
		Error("Log", "Unknown level %d.", l);
	} break;
	}
	return "Unknown";
}

void LogPrintVarArgs(str::String file, str::String func, s64 line, LogLevel lvl, str::String category, str::String fmt, va_list args)
{
	#if DebugBuild
		auto msg = str::FormatVarArgs(fmt, args);
		// We need to be a bit careful about not allocating memory, because this might be called using
		// the fixed-size backup allocator.
		if (l >= CurrentLogLevel())
		{
			ConsoleWrite("[");
			ConsoleWrite(category);
			ConsoleWrite("] ");
			ConsoleWrite(msg);
			ConsoleWrite("\n");
		}
		auto lf = LogFile();
		if (lf->IsOpen())
		{
			lf->WriteString("[");
			lf->WriteString(category);
			lf->WriteString("] ");
			lf->WriteString(LogLevelToString(l));
			//sb.FormatTime();
			lf->WriteString(str::Format("%k:%d %k  |  ", file, line, func));
			lf->WriteString(msg);
			lf->WriteString("\n");
		}
	#endif
}

void LogPrintActual(str::String file, str::String func, s64 line, LogLevel l, str::String category, str::String fmt, ...)
{
	#if DebugBuild
		va_list args;
		va_start(args, fmt);
		LogPrintVarArgs(file, func, line, l, category, fmt, args);
		va_end(args);
	#endif
}

void LogPrintActual(const char *file, const char *func, s64 line, LogLevel l, const char *category, const char *fmt, ...)
{
	#if DebugBuild
		va_list args;
		va_start(args, fmt);
		LogPrintVarArgs(file, func, line, l, category, fmt, args);
		va_end(args);
	#endif
}

File NewCrashLogFile()
{
	auto sb = str::Builder{};
	sb.Append(LogFileDirectory());
	sb.Append("Crash/Crash");
	sb.FormatTime();
	sb.Append(".txt");
	auto err = false;
	auto path = sb.View(0, sb.Length());
	return OpenFile(path,  OpenFileCreate | OpenFileWriteOnly, &err);
}

void DoAbortActual(str::String file, str::String func, s64 line, str::String category, str::String fmt, va_list args)
{
	// In the case of a recursive abort, just ignore the recursion.
	static auto aborting = s64{0};
	if (AtomicCompareAndSwap64(&aborting, 0, 1) != 0)
	{
		return;
	}
	// We have to be a bit careful about logging, as this function can get called during global heap
	// initialization, in which case will be using a fixed-size stack allocator. So we should try
	// not to overflow the stack allocator.
	// This is pretty ugly and I kind of hate it.
	auto st = Stacktrace();
	// @TODO
	auto pool = mem::NewPoolAllocator(8 * Kilobyte, 1, mem::GlobalHeap(), mem::GlobalHeap());
	//SetContextAllocator(&pool);
	//contextAllocator = &pool; // @TODO
	Fatal(category, "###########################################################################");
	Fatal(category, "[ABORT]");
	LogPrintVarArgs(file, func, line, FatalLog, category, fmt, args);
	Fatal(category, "###########################################################################");
	//pool.Clear();
	auto crashLog = NewCrashLogFile();
	if (crashLog.IsOpen())
	{
		auto msg = str::Builder{};
		msg.FormatVarArgs(fmt, args);
		crashLog.WriteString("###########################################################################\n");
		crashLog.WriteString("[ABORT]\n");
		crashLog.Write(msg.buffer);
		crashLog.WriteString("\n");
		crashLog.WriteString("###########################################################################\n");
		for (auto ch : crashHandlers)
		{
			ch(crashLog);
		}
		crashLog.WriteString("Stack trace:\n");
		for (auto s : st)
		{
			crashLog.WriteString("\t");
			crashLog.WriteString(s);
			crashLog.WriteString("\n");
		}
	}
	//pool.Clear();
	if (IsDebuggerAttached())
	{
		SignalDebugBreakpoint();
	}
	LogFatal(category, "Stack trace:");
	for (auto i = 0; i < st.count; i += 1)
	{
		LogFatal(category, "%k", i, st[i]);
	}
	ExitProcess(ProcessFail);
}

void AbortActual(str::String file, str::String func, s64 line, str::String category, str::String fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	DoAbortActual(file, func, line, category, fmt, args);
}

void AbortActual(const char *file, const char *func, s64 line, const char *category, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	DoAbortActual(file, func, line, category, fmt, args);
}
