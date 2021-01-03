#pragma once

#ifdef __linux__
	#ifdef __x86_64__
		#include "CPU_Linux_X64.h"
	#else
		#error Unsupported CPU.
	#endif
#else
	#error Unsupported operating system.
#endif
