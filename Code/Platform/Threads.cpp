#if defined(__linux__)
	#include "Platform/Linux/Threads.cpp"
#else
	#error unsupported platform
#endif

__thread u32 threadIndex;
