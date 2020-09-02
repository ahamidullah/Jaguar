#pragma once

#include "../String.h"
#include "../Array.h"
#include "Common.h"

enum ProcessExitCode
{
	ProcessSuccess = 0,
	ProcessFail = 1,
};

s32 RunProcess(String cmd);
void ExitProcess(ProcessExitCode c);
bool IsDebuggerAttached();
void SignalDebugBreakpoint();
String EnvironmentVariable(String name, bool *exists);
Array<String> Stacktrace();
