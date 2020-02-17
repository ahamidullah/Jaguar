#include <errno.h>
#include <string.h>

void PrintStacktrace()
{
	LogPrint(LogType::INFO, "Stack trace:\n");

	const u32 addressBufferSize = 100;
	void *addresses[addressBufferSize];
	s32 addressCount = backtrace(addresses, addressBufferSize);
	if (addressCount == addressBufferSize)
	{
		LogPrint(LogType::ERROR, "stack trace is probably truncated.\n");
	}

	char **strings = backtrace_symbols(addresses, addressCount);
	if (!strings)
	{
		LogPrint(LogType::ERROR, "failed to get function names\n");
		return;
	}
	for (s32 i = 0; i < addressCount; i++)
	{
		LogPrint(LogType::INFO, "\t%s\n", strings[i]);
	}
	free(strings);
}

const char *GetPlatformError()
{
	return strerror(errno);
}
