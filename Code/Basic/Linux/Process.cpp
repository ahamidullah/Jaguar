#include <unistd.h>
#include <signal.h>

void ExitProcess(ProcessExitCode exitCode) {
	_exit((s32)exitCode);
}

void SignalDebugBreakpoint() {
	raise(SIGTRAP);
}

s32 RunProcess(const String &command)
{
	return system(&command[0]);
}

String GetEnvironmentVariable(const String &name, bool *exists)
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
