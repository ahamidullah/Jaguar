#include "../Process.h"
#include "../PCH.h"
#include "../Log.h"

s32 RunProcess(String cmd)
{
	return system(cmd.CString());
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

String EnvironmentVariable(String name, bool *exists)
{
	auto result = getenv(name.CString());
	if (!result)
	{
		*exists = false;
		return "";
	}
	*exists = true;
	return result;
}

Array<String> Stacktrace()
{
	#if DebugBuild
		const auto maxAddrs = 100;
		auto addrs = StaticArray<void *, maxAddrs>{};
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
		auto st = NewArrayWithCapacity<String>(numAddrs);
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
