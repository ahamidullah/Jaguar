#include "../Process.h"

void ExitProcess(ProcessExitCode c)
{
	_exit((s32)c);
}

void SignalDebugBreakpoint()
{
	raise(SIGTRAP);
}

s32 RunProcess(const String &cmd)
{
	return system(&cmd[0]);
}

String GetEnvironmentVariable(String name, bool *exists)
{
	auto result = getenv(&name[0]);
	if (!result)
	{
		*exists = false;
		return "";
	}
	*exists = true;
	return result;
}
