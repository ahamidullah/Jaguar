#pragma once

#include "Basic/String.h"
#include "Basic/Container/Array.h"
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
array::Array<String> Stacktrace();
