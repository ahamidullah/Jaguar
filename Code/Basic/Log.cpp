#include "Log.h"
#include "File.h"
#include "Process.h"
#include "Thread.h"
#include "Atomic.h"
#include "Memory/GlobalHeap.h"
#include "Memory/PoolAllocator.h"

// We want to allow global variable initialization to use these function, so we have to be a bit
// careful about how we do things.
// Global variables are not allowed, because we can't control initialization order.
// Also, these functions my be called before the global heap is initialized, including if the global
// heap fails to initialize, so we can't totally rely on the global heap!

typedef void (*CrashHandler)(File crashLog);

auto crashHandlers = array::NewIn<CrashHandler>(Memory::GlobalHeap(), 0);

string::String LogFileDirectory()
{
	return "Data/Log/";
}

auto logLevel = InfoLog;

LogLevel CurrentLogLevel()
{
	return logLevel;
}

void SetLogLevel(LogLevel l)
{
	logLevel = l;
}

File *LogFile()
{
	static auto initLock = Spinlock{};
	static auto init = false;
	static auto f = File{};
	initLock.Lock();
	if (!init)
	{
		// @TODO
		init = true;
	}
	initLock.Unlock();
	return &f;
}

// @TODO
// Log: Was initialized.
//      Log file(s) state.
//      Crash handler state.
//      Allocator state.
void LogCrashHandler(File crashLog)
{
}

void ConsolePrintVarArgs(string::String fmt, va_list args)
{
	auto sb = string::Builder{};
	sb.FormatVarArgs(fmt, args);
	ConsoleWrite(sb.ToView(0, sb.Length()));
}

void ConsolePrint(string::String fmt, ...)
{
	#if DebugBuild
		va_list args; // Clang complains if we use 'auto' style declartions with va_list for some reason...
		va_start(args, fmt);
		ConsolePrintVarArgs(fmt, args);
		va_end(args);
	#endif
}

void ConsolePrintCtx(string::String fmt, ...)
{
	#if DebugBuild
		va_list args; // Clang complains if we use 'auto' style declartions with va_list for some reason...
		va_start(args, fmt);
		ConsolePrintVarArgs(fmt, args);
		va_end(args);
	#endif
}

string::String LogLevelToString(LogLevel l)
{
	switch (l)
	{
	case VerboseLog:
	{
		return "Verbose";
	} break;
	case InfoLog:
	{
		return "Info";
	} break;
	case ErrorLog:
	{
		return "Error";
	} break;
	case FatalLog:
	{
		return "Fatal";
	} break;
	default:
	{
		LogError("Log", "Unknown log level %d.", l);
	} break;
	}
	return "Unknown";
}

void LogPrintVarArgs(string::String file, string::String func, s64 line, LogLevel l, string::String category, string::String fmt, va_list args)
{
	#if DebugBuild
		auto msg = string::FormatVarArgs(fmt, args);
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
			lf->WriteString(string::Format("%k:%d %k  |  ", file, line, func));
			lf->WriteString(msg);
			lf->WriteString("\n");
		}
	#endif
}

void LogPrintActual(string::String file, string::String func, s64 line, LogLevel l, string::String category, string::String fmt, ...)
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
	auto sb = string::Builder{};
	sb.Append(LogFileDirectory());
	sb.Append("Crash/Crash");
	sb.FormatTime();
	sb.Append(".txt");
	auto err = false;
	auto path = sb.ToView(0, sb.Length());
	return OpenFile(path,  OpenFileCreate | OpenFileWriteOnly, &err);
}

void DoAbortActual(string::String file, string::String func, s64 line, string::String category, string::String fmt, va_list args)
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
	auto pool = Memory::NewPoolAllocator(8 * Kilobyte, 1, Memory::GlobalHeap(), Memory::GlobalHeap());
	//SetContextAllocator(&pool);
	//contextAllocator = &pool; // @TODO
	LogFatal(category, "###########################################################################");
	LogFatal(category, "[ABORT]");
	LogPrintVarArgs(file, func, line, FatalLog, category, fmt, args);
	LogFatal(category, "###########################################################################");
	//pool.Clear();
	auto crashLog = NewCrashLogFile();
	if (crashLog.IsOpen())
	{
		auto msg = string::Builder{};
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

void AbortActual(string::String file, string::String func, s64 line, string::String category, string::String fmt, ...)
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
