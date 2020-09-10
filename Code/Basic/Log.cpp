#include "Log.h"
#include "String.h"
#include "File.h"
#include "Memory.h"
#include "Process.h"
#include "Thread.h"
#include "Atomic.h"

// We want to allow global variable initialization to use these function, so we have to be a bit
// careful about how we do things.
// Global variables are not allowed, because we can't control initialization order.
// Also, these functions my be called before the global heap is initialized, including if the global
// heap fails to initialize, so we can't totally rely on the global heap!

typedef void (*CrashHandler)(File crashLog);

auto crashHandlers = NewArrayIn<CrashHandler>(GlobalHeap(), 0);

String LogFileDirectory()
{
	return String{"Data/Log/"};
}

LogLevel *LogLevelPointer()
{
	static auto logLevel = InfoLog;
	return &logLevel;
}

LogLevel CurrentLogLevel()
{
	return *LogLevelPointer();
}

void SetLogLevel(LogLevel l)
{
	*LogLevelPointer() = l;
}

File *LogFile()
{
	static auto init = false;
	static auto f = File{};
	if (!init)
	{
		// @TODO
		init = true;
	}
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

#if 0
void InitializeLog()
{
	#ifdef DebugBuild
		/*
		Assert(IsGlobalHeapInitialized());
		auto sb = StringBuilder{};
		Defer(sb.Free());
		sb.Append(LogFileDirectory());
		sb.FormatTime();
		auto dir = sb.ToString();
		Defer(dir.Free());
		return; // @TODO
		if (!CreateDirectory(dir))
		{
			LogError("Log", "Failed to create log directory %k.\n", dir);
			LogInfo("Log", "Initialized logging.\n");
			return;
		}
		sb.Append("/");
		sb.Append("Global.log");
		auto logPath = sb.ToString();
		auto err = false;
		logFile = OpenFile(logPath, OpenFileWriteOnly | OpenFileCreate, &err);
		if (err)
		{
			LogError("Log", "Failed to open global log file %k.\n", logPath);
		}
		LogInfo("Log", "Initialized logging.\n");
		*/
	#endif
}
#endif

void ConsolePrintVarArgs(String fmt, va_list args)
{
	auto sb = StringBuilder{};
	Defer(sb.Free());
	sb.FormatVarArgs(fmt, args);
	ConsoleWrite(sb.ToView(0, sb.Length()));
}

void ConsolePrint(String fmt, ...)
{
	#ifdef DebugBuild
		va_list args; // Clang complains if we use 'auto' style declartions with va_list for some reason...
		va_start(args, fmt);
		ConsolePrintVarArgs(fmt, args);
		va_end(args);
	#endif
}

void ConsolePrintCtx(String fmt, ...)
{
	#ifdef DebugBuild
		va_list args; // Clang complains if we use 'auto' style declartions with va_list for some reason...
		va_start(args, fmt);
		ConsolePrintVarArgs(fmt, args);
		va_end(args);
	#endif
}

String LogLevelToString(LogLevel l)
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
		Abort("Log", "Unknown log level %d.", l);
	} break;
	}
	return "Unknown";
}

void LogPrintVarArgs(String file, String func, s64 line, LogLevel l, String category, String fmt, va_list args)
{
	#ifdef DebugBuild
		auto msgSB = StringBuilder{};
		Defer(msgSB.Free());
		msgSB.FormatVarArgs(fmt, args);
		auto msg = msgSB.ToView(0, msgSB.Length());
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
		if (LogFile()->IsOpen())
		{
			LogFile()->WriteString("[");
			LogFile()->WriteString(category);
			LogFile()->WriteString("] ");
			LogFile()->WriteString(LogLevelToString(l));
			//sb.FormatTime();
			auto infoSB = StringBuilder{};
			Defer(infoSB.Free());
			infoSB.Format("%k:%d %k  |  ", file, line, func);
			LogFile()->WriteString(infoSB.ToView(0, infoSB.Length()));
			LogFile()->WriteString(msg);
			LogFile()->WriteString("\n");
		}
	#endif
}

void LogPrintActual(String file, String func, s64 line, LogLevel l, String category, String fmt, ...)
{
	#ifdef DebugBuild
		va_list args;
		va_start(args, fmt);
		LogPrintVarArgs(file, func, line, l, category, fmt, args);
		va_end(args);
	#endif
}

void LogPrintActual(const char *file, const char *func, s64 line, LogLevel l, const char *category, const char *fmt, ...)
{
	#ifdef DebugBuild
		va_list args;
		va_start(args, fmt);
		LogPrintVarArgs(file, func, line, l, category, fmt, args);
		va_end(args);
	#endif
}

File NewCrashLogFile()
{
	auto sb = StringBuilder{};
	sb.Append(LogFileDirectory());
	sb.Append("Crash/Crash");
	sb.FormatTime();
	sb.Append(".txt");
	auto err = false;
	auto path = sb.ToString();
	return OpenFile(path,  OpenFileCreate | OpenFileWriteOnly, &err);
}

// @TODO: Test a really long abort message during global heap initialization (should just truncate the error message when we run out of stack space).
void DoAbortActual(String file, String func, s64 line, String category, String fmt, va_list args)
{
	static auto aborting = s64{0};
	if (AtomicCompareAndSwap64(&aborting, 0, 1) != 0)
	{
		return;
	}
	// We have to be a bit careful about logging, as this function can get called during global heap
	// initialization, in which case will be using a fixed-size stack allocator. So we should try
	// not to overflow the stack allocator.
	auto st = Stacktrace();
	auto pool = NewPoolAllocator(KilobytesToBytes(8), 1, ContextAllocator(), ContextAllocator());
	SetContextAllocator(&pool);
	LogFatal(category, "###########################################################################");
	LogFatal(category, "[ABORT]");
	LogPrintVarArgs(file, func, line, FatalLog, category, fmt, args);
	LogFatal(category, "###########################################################################");
	pool.Clear();
	auto crashLog = NewCrashLogFile();
	if (crashLog.IsOpen())
	{
		auto msg = StringBuilder{};
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
	pool.Clear();
	if (IsDebuggerAttached())
	{
		SignalDebugBreakpoint();
	}
	LogFatal(category, "Stack trace:");
	for (auto i = 0; i < st.count; i++)
	{
		LogFatal(category, "\t%d: %k", i, st[i]);
	}
	ExitProcess(ProcessFail);
}

void AbortActual(String file, String func, s64 line, String category, String fmt, ...)
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
