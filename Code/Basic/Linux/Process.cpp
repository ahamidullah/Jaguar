#include "../Process.h"
#include "../PCH.h"
#include "../Log.h"

s32 RunProcess(string::String cmd)
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

string::String EnvironmentVariable(string::String name, bool *exists)
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

array::Array<string::String> Stacktrace()
{
	#if DebugBuild
		const auto maxAddrs = 100;
		auto addrs = array::Static<void *, maxAddrs>{};
		auto numAddrs = backtrace(&addrs[0], maxAddrs);
		if (numAddrs == maxAddrs)
		{
			LogError("Memory", "Stack trace is probably truncated.\n");
		}
		auto trace = backtrace_symbols(&addrs[0], numAddrs);
		if (!trace)
		{
			LogError("Memory", "Failed to get stack trace function names.\n");
			return {};
		}
		auto st = array::NewWithCapacity<string::String>(numAddrs);
		for (auto i = 0; i < numAddrs; i += 1)
		{
			st.Append(trace[i]);
		}
		free(trace);
		return st;
	#else
		return {};
	#endif
}
