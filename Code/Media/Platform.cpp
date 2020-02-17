#if defined(__linux__)
	#include "Platform/Linux/Platform.cpp"
#else
	#error unsupported platform
#endif
