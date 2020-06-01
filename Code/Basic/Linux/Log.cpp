#include "../Log.h"
#include "../File.h"

void WriteToConsole(const String &writeString)
{
	WriteToFile(1, StringLength(writeString), &writeString[0]);
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
		LogPrint(ERROR_LOG, "Failed to get function names.\n");
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
