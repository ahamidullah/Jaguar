#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#if defined(__linux__)
	// DLL
	#include <dlfcn.h>

	// Fiber
	#include <ucontext.h>
	#include <setjmp.h>

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
