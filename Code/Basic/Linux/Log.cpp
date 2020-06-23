#include "../Log.h"
#include "../File.h"
#include "../Process.h"

void WriteToConsole(String s)
{
	WriteToFile(File{1}, s.length, &s[0]);
}

void PrintStacktrace()
{
	LogPrint(LogLevelInfo, "Log", "Stack trace:\n");
	auto maxAddrs = 100;
	void *addrs[maxAddrs];
	auto numAddrs = backtrace(addrs, maxAddrs);
	if (numAddrs == maxAddrs)
	{
		LogPrint(LogLevelError, "Log", "Stack trace is probably truncated.\n");
	}
	auto **trace = backtrace_symbols(addrs, numAddrs);
	if (!trace)
	{
		LogPrint(LogLevelError, "Log", "Failed to get stack trace function names.\n");
		return;
	}
	for (auto i = 0; i < numAddrs; i++)
	{
		LogPrint(LogLevelInfo, "Log", "\t%s\n", trace[i]);
	}
	free(trace);
}

String GetPlatformError()
{
	return strerror(errno);
}
