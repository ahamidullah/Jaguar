#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <xmmintrin.h>
#ifdef ThreadSanitizerBuild
	#include <sanitizer/tsan_interface.h>
#endif
#include <string.h> // memcpy @TODO @DELETEME
#include <stdlib.h> // realloc @TODO @DELETEME
#include <alloca.h>

#ifdef __linux__
	// DLL
	#include <dlfcn.h>

	// Fiber
	#include <sys/mman.h>

	// File
	#include <sys/stat.h>
	#include <dirent.h>
	#include <fcntl.h>
	
	// Log
	#include <errno.h>
	#include <string.h>

	// Memory
	#include <execinfo.h>
	#include <sys/mman.h>
	#include <unistd.h>

	// Thread
	#include <pthread.h>
	#include <sys/syscall.h>
	#include <sys/prctl.h>

	// Semaphore
	#include <semaphore.h>

	// Process
	#include <unistd.h>
	#include <signal.h>
	#include <sys/ptrace.h>

	// Time
	#include <time.h>
#else
	#error Unsupported platform.
#endif
