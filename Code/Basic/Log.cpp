#include "Log.h"
#include "String.h"
#include "File.h"
#include "Memory.h"
#include "Process.h"

typedef void (*CrashHandler)(File crashLog);

#ifdef DebugBuild
	auto LogFileDir = String{"Data/Log/"};

	auto logLock = Spinlock{};
	auto logFile = File{};
	auto logLevel = InfoLog;
	auto logPool = PoolAllocator{};
#endif
auto crashHandlers = NewArrayIn<CrashHandler>(GlobalHeap(), 0);

// @TODO
// Log: Was initialized.
//      Lock state.
//      Log file(s) state.
//      Crash handler state.
//      Allocator state.
void LogCrashHandler(File crashLog)
{
}

void InitializeLog()
{
	#ifdef DebugBuild
		logPool = NewPoolAllocator(KilobytesToBytes(2), 1, GlobalHeap(), GlobalHeap());
		PushContextAllocator(&logPool);
		Defer(
		{
			PopContextAllocator();
			logPool.Clear();
		});
		auto sb = StringBuilder{};
		sb.Append(LogFileDir);
		sb.FormatTime();
		auto dir = sb.ToString();
		return;
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
	#endif
}

void ConsolePrintVarArgs(String fmt, va_list args)
{
	#ifdef DebugBuild
		PushContextAllocator(&logPool);
		Defer(
		{
			PopContextAllocator();
			logPool.Clear();
		});
		auto sb = StringBuilder{};
		sb.FormatVarArgs(fmt, args);
		ConsoleWrite(sb.ToString());
	#endif
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

void ConsolePrint(const char *fmt, ...)
{
	#ifdef DebugBuild
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
		Abort("Unknown log level %d.", l);
	} break;
	}
	return "Unknown";
}

void LogPrintVarArgs(String file, String func, s64 line, LogLevel l, String category, String fmt, va_list args)
{
	#ifdef DebugBuild
		PushContextAllocator(&logPool);
		Defer(
		{
			PopContextAllocator();
			logPool.Clear();
		});
		auto sb = StringBuilder{};
		sb.FormatVarArgs(fmt, args);
		auto msg = sb.ToString();
		sb.Resize(0);
		if (logLevel >= l)
		{
			ConsolePrint("[%k] ", category);
			ConsolePrint(msg);
			ConsolePrint("\n");
		}
		if (logFile.IsOpen())
		{
			sb.Format("[%k] ", category);
			sb.Format("%k ", LogLevelToString(l));
			sb.FormatTime();
			sb.Format("%k:%d %k  |  ", file, line, func);
			logFile.Write(sb.buffer);
			logFile.WriteString(msg);
			logFile.WriteString("\n");
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

void LogPrintActual(const char *file, const char *func, s64 line, LogLevel l, String category, const char *fmt, ...)
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
	sb.Append(LogFileDir);
	sb.Append("Crash/Crash");
	sb.FormatTime();
	sb.Append(".txt");
	auto err = false;
	auto path = sb.ToString();
	auto f = OpenFile(path,  OpenFileCreate | OpenFileWriteOnly, &err);
	if (err)
	{
		LogError("Log", "Failed to open crash log file %k.\n", path);
	}
	return f;
}

void DoAbortActual(String file, String func, s64 line, String fmt, va_list args)
{
	LogFatal("Global", "###########################################################################\n");
	LogFatal("Global", "[ABORT]\n");
	LogPrintVarArgs(file, func, line, FatalLog, "Global", fmt, args);
	LogFatal("Global", "###########################################################################\n");
	auto crashLog = NewCrashLogFile();
	auto sb = StringBuilder{};
	sb.FormatVarArgs(fmt, args);
	crashLog.WriteString("###########################################################################\n");
	crashLog.WriteString("[ABORT]\n");
	crashLog.Write(sb.buffer);
	crashLog.WriteString("\n");
	crashLog.WriteString("###########################################################################\n");
	for (auto ch : crashHandlers)
	{
		ch(crashLog);
	}
	if (IsDebuggerAttached())
	{
		SignalDebugBreakpoint();
	}
	auto st = Stacktrace();
	ConsolePrint("Stack trace:\n");
	for (auto s : st)
	{
		ConsolePrint("\t");
		ConsolePrint(s);
		ConsolePrint("\n");
	}
	crashLog.WriteString("Stack:\n");
	for (auto s : st)
	{
		crashLog.WriteString("\t");
		crashLog.WriteString(s);
		crashLog.WriteString("\n");
	}
	crashLog.Close();
	ExitProcess(ProcessFail);
}

void AbortActual(String file, String func, s64 line, String fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	DoAbortActual(file, func, line, fmt, args);
}

void AbortActual(const char *file, const char *func, s64 line, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	DoAbortActual(file, func, line, fmt, args);
}
