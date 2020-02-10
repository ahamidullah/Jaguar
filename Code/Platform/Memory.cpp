#if defined(__linux__)
	#include "Platform/Linux/Memory.cpp"
#else
	#error unsupported platform
#endif
