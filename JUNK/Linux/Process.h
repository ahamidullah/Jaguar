#pragma once

#include "Code/Common.h"

enum ProcessExitCode
{
	PROCESS_EXIT_SUCCESS = 0,
	PROCESS_EXIT_FAILURE = 1,
};

struct String;

s32 RunProcess(const String &command);
void ExitProcess(ProcessExitCode exitCode);
void SignalDebugBreakpoint();
String GetEnvironmentVariable(const String &name, bool *exists);