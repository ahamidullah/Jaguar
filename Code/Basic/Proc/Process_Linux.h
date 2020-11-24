#pragma once

#include "Basic/String.h"
#include "Basic/Container/Array.h"
#include "Common.h"

namespace process
{

enum class ExitStatus
{
	Success = 0,
	Fail = 1,
};

s32 Run(string::String cmd);
void Exit(ExitStatus s);
bool IsDebuggerAttached();
void SignalBreakpoint();
string::String EnvironmentVariable(string::String name, bool *exists);
array::Array<string::String> Stacktrace();

}
