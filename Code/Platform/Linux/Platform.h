#pragma once

constexpr s32 PLATFORM_FAILURE_EXIT_CODE = 1;
constexpr s32 PLATFORM_SUCCESS_EXIT_CODE = 0;

#if 0
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <ucontext.h>
#include <setjmp.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

constexpr s32 PLATFORM_FAILURE_EXIT_CODE = 1;
constexpr s32 PLATFORM_SUCCESS_EXIT_CODE = 0;

constexpr s32 PLATFORM_STANDARD_OUT = 1;
constexpr s32 PLATFORM_STANDARD_IN = 0;
constexpr s32 PLATFORM_STANDARD_ERROR = 2;

#define THREAD_LOCAL __thread

struct PlatformFiber {
	ucontext_t context;
	jmp_buf jumpBuffer;
};

typedef s32 PlatformFileHandle;
typedef u64 PlatformFileOffset;
typedef struct timespec PlatformTime;
typedef void *PlatformDynamicLibraryHandle;
typedef void *PlatformDynamicLibraryFunction;
typedef pthread_t PlatformThreadHandle;
typedef void *(*PlatformThreadProcedure)(void *);
typedef void (*PlatformFiberProcedure)(void *);
typedef sem_t PlatformSemaphore;
typedef pthread_mutex_t PlatformMutex;

enum PlatformKeySymbol {
	W_KEY = XK_w,
	A_KEY = XK_a,
	S_KEY = XK_s,
	D_KEY = XK_d,
	E_KEY = XK_e,
	G_KEY = XK_g,
	Q_KEY = XK_q,
	R_KEY = XK_r,
	F_KEY = XK_f,
	P_KEY = XK_p,
	L_KEY = XK_l,
	C_KEY = XK_c,
	J_KEY = XK_j,
	K_KEY = XK_k,
	I_KEY = XK_i,
	M_KEY = XK_m,
	BACKSPACE_KEY = XK_BackSpace,
	LCTRL_KEY = XK_Control_L,
	RCTRL_KEY = XK_Control_R,
	LALT_KEY = XK_Alt_L,
	RALT_KEY = XK_Alt_R,
	ESCAPE_KEY = XK_Escape,
};

enum PlatformMouseButton {
	MOUSE_BUTTON_LEFT = 0,
	MOUSE_BUTTON_MIDDLE,
	MOUSE_BUTTON_RIGHT,
};

enum PlatformFileSeekRelative {
	FILE_SEEK_START = SEEK_SET,
	FILE_SEEK_CURRENT = SEEK_CUR,
	FILE_SEEK_END = SEEK_END
};

enum PlatformOpenFileFlags {
	OPEN_FILE_READ_ONLY = O_RDONLY,
};

PlatformFileHandle FILE_HANDLE_ERROR = -1;
PlatformFileOffset FILE_OFFSET_ERROR = (PlatformFileOffset)-1;

struct PlatformDirectoryIteration {
	DIR *dir;
	struct dirent *dirent;
	char *filename;
	u8 is_directory;
};
#endif
