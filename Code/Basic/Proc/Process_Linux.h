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

s32 Run(str::String cmd);
void Exit(ExitStatus s);
bool IsDebuggerAttached();
void SignalBreakpoint();
str::String EnvironmentVariable(str::String name, bool *exists);
arr::array<str::String> Stacktrace();

}
