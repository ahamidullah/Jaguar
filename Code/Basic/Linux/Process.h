#pragma once

enum struct ProcessExitCode
{
	SUCCESS = 0,
	FAILURE = 1,
};

struct String;

s32 RunProcess(const String &command);
void ExitProcess(ProcessExitCode exitCode);
void SignalDebugBreakpoint();
