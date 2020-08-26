#include "../Process.h"

s32 RunProcess(String cmd)
{
	return system(cmd.ToCString());
}

void ExitProcess(ProcessExitCode c)
{
	_exit((s32)c);
}

bool IsDebuggerAttached()
{
	if (ptrace(PTRACE_TRACEME, 0, 1, 0) == -1)
	{
		return true;
	}
	return false;
}

void SignalDebugBreakpoint()
{
	raise(SIGTRAP);
}

String EnvironmentVariable(String name, bool *exists)
{
	auto result = getenv(name.ToCString());
	if (!result)
	{
		*exists = false;
		return "";
	}
	*exists = true;
	return result;
}
