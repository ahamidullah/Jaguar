#include "Platform.h"

#if defined(__linux__)
	#include "Platform/Linux/Linux.cpp"
#else
	#error unsupported platform
#endif

