#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <xmmintrin.h>
#if defined(THREAD_SANITIZER_BUILD)
	#include <sanitizer/tsan_interface.h>
#endif
#include <string.h> // memcpy @TODO @DELETEME
#include <stdlib.h> // realloc @TODO @DELETEME
#include <new> // placement new @TODO @DELETEME

#if defined(__linux__)
	// DLL
	#include <dlfcn.h>

	// Fiber
	#include <ucontext.h>
	#include <setjmp.h>
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

	// Time
	#include <time.h>
#else
	#error Unsupported platform.
#endif
