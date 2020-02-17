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
