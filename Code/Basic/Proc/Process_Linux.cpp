#include "../Process.h"
#include "../PCH.h"
#include "../Log.h"

namespace proc
{

s32 Run(str::String cmd)
{
	return system(cmd.ToCString());
}

void Exit(ExitStatus s)
{
	_exit(s32(s));
}

bool IsDebuggerAttached()
{
	if (ptrace(PTRACE_TRACEME, 0, 1, 0) == -1)
	{
		return true;
	}
	return false;
}

void SignalBreakpoint()
{
	raise(SIGTRAP);
}

str::String EnvironmentVariable(str::String name, bool *exists)
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

arr::Array<str::String> Stacktrace()
{
	#if DebugBuild
		const auto maxAddrs = 100;
		auto addrs = array::Static<void *, maxAddrs>{};
		auto numAddrs = backtrace(&addrs[0], maxAddrs);
		if (numAddrs == maxAddrs)
		{
			log::Error("Memory", "Stack trace is probably truncated.\n");
		}
		auto trace = backtrace_symbols(&addrs[0], numAddrs);
		if (!trace)
		{
			log::Error("Memory", "Failed to get stack trace function names.\n");
			return {};
		}
		auto st = arr::NewWithCapacity<str::String>(numAddrs);
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

}
