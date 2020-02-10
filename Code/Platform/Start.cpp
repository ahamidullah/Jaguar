#if defined(__linux__)
	#include "Platform/Linux/Start.cpp"
#else
	#error unsupported platform
#endif
