#pragma once

enum ProcessExitCode
{
	PROCESS_SUCCESS = 0,
	PROCESS_FAILURE = 1,
};

struct String;

s32 RunProcess(const String &command);
void ExitProcess(ProcessExitCode exitCode);
void SignalDebugBreakpoint();
String GetEnvironmentVariable(const String &name, bool *exists);
