#include "../Basic.h"

void WriteToConsole(String s)
{
	WriteToFile(File{1}, s.length, &s[0]);
}

void PrintStacktrace()
{
	LogPrint(INFO_LOG, "Stack trace:\n");
	auto addressBufferSize = 100;
	void *addresses[addressBufferSize];
	auto addressCount = backtrace(addresses, addressBufferSize);
	if (addressCount == addressBufferSize)
	{
		LogPrint(ERROR_LOG, "Stack trace is probably truncated.\n");
	}
	auto **strings = backtrace_symbols(addresses, addressCount);
	if (!strings)
	{
		LogPrint(ERROR_LOG, "Failed to get stack trace function names.\n");
		return;
	}
	for (auto i = 0; i < addressCount; i++)
	{
		LogPrint(INFO_LOG, "\t%s\n", strings[i]);
	}
	free(strings);
}

String GetPlatformError()
{
	return strerror(errno);
}
