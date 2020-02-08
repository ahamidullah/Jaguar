#include "Platform.h"

#include "Jaguar.cpp"

#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/XInput2.h>
#define ALSA_PCM_NEW_HW_PARAMS_API
//#include <alsa/asoundlib.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sched.h>
#include <time.h>
#include <signal.h>
#include <dlfcn.h>
#include <errno.h>
#include <execinfo.h>

// @TODO: Store colormap and free it on exit.
// @TODO: Free blankCursor.
struct {
	Display *display;
	Window window;
	Atom deleteWindowAtom;
	s32 xinputOpcode;
	Cursor blankCursor;
	char *fiberStackMemory;
	volatile s32 fiberCount;
	size_t pageSize;
} linuxContext;

THREAD_LOCAL struct {
	PlatformFiber threadFiber;
	PlatformFiber *activeFiber;
} threadLocalLinuxContext;

////////////////////////////////////////
//
// Process.
//

void PlatformExitProcess(s32 exitCode) {
	_exit(exitCode);
}

////////////////////////////////////////
//
// Input.
//

u32 PlatformKeySymbolToScancode(PlatformKeySymbol keySymbol) {
	u32 scanCode = XKeysymToKeycode(linuxContext.display, keySymbol);
	Assert(scanCode > 0);
	return scanCode;
}

void PlatformGetMousePosition(s32 *x, s32 *y) {
	s32 screenX, screenY;
	Window root, child;
	u32 mouseButtons;
	XQueryPointer(linuxContext.display, linuxContext.window, &root, &child, &screenX, &screenY, x, y, &mouseButtons);
	*y = (window_height - *y); // Bottom left is zero for us, top left is zero for x11.
}

////////////////////////////////////////
//
// Dynamic libraries.
//

PlatformDynamicLibraryHandle PlatformOpenDynamicLibrary(const char *filename) {
	void* library = dlopen(filename, RTLD_NOW | RTLD_LOCAL);
	if (!library) {
		Abort("Failed to load shared library: %s", dlerror());
	}
	return library;
}

void PlatformCloseDynamicLibrary(PlatformDynamicLibraryHandle library) {
	s32 errorCode = dlclose(library);
	if (errorCode < 0) {
		LogPrint(ERROR_LOG, "Failed to close shared library: %s\n", dlerror());
	}
}

PlatformDynamicLibraryFunction PlatformGetDynamicLibraryFunction(PlatformDynamicLibraryHandle library, const char *functionName) {
	void *function = dlsym(library, functionName);
	if (!function) {
		Abort("Failed to load shared library function %s", functionName);
	}
	return function;
}

////////////////////////////////////////
//
// Time.
//

PlatformTime PlatformGetCurrentTime() {
	PlatformTime time;
	clock_gettime(CLOCK_MONOTONIC_RAW, &time);
	return time;
}

// Time in milliseconds.
f64 PlatformTimeDifference(PlatformTime start, PlatformTime end) {
	return ((end.tv_sec - start.tv_sec) * 1000.0) + ((end.tv_nsec - start.tv_nsec) / 1000000.0);
}

void PlatformSleep(u32 milliseconds) {
	struct timespec timespec = {
		.tv_sec = milliseconds / 1000,
		.tv_nsec = (milliseconds % 1000) * 1000000,
	};
	if (nanosleep(&timespec, NULL)) {
		LogPrint(ERROR_LOG, "nanosleep() ended early: %s.", PlatformGetError());
	}
}

////////////////////////////////////////
//
// Filesystem.
//

// @TODO: Handle modes.
PlatformOpenFileResult PlatformOpenFile(const String &path, PlatformOpenFileFlags flags) {
	PlatformFileHandle file = open(&path[0], flags, 0666);
	if (file < 0) {
		LogPrint(ERROR_LOG, "could not open file: %s", &path[0]);
		return {.error = true};
	}
	return {.file = file};
}

bool PlatformCloseFile(PlatformFileHandle file) {
	s32 result = close(file);
	if (result == -1) {
		LogPrint(ERROR_LOG, "Could not close file: %s", PlatformGetError());
		return false;
	}
	return true;
}

PlatformReadFileResult PlatformReadFromFile(PlatformFileHandle file, size_t numberOfBytesToRead) {
	size_t totalBytesRead = 0;
	ssize_t currentBytesRead = 0; // Maximum number of bytes that can be returned by a read. (Like size_t, but signed.)
	auto result = CreateString(numberOfBytesToRead);
	auto *cursor = &result[0];
	do {
		currentBytesRead = read(file, cursor, numberOfBytesToRead - totalBytesRead);
		totalBytesRead += currentBytesRead;
		cursor += currentBytesRead;
	} while (totalBytesRead < numberOfBytesToRead && currentBytesRead != 0 && currentBytesRead != -1);
	if (currentBytesRead == -1) {
		LogPrint(ERROR_LOG, "ReadFromFile failed: could not read from file: %s", PlatformGetError());
		PlatformReadFileResult x;
		x.error = true;
		return x;
		//return {.error = true};
	} else if (totalBytesRead != numberOfBytesToRead) {
		// @TODO: Add file name to file handle.
		LogPrint(ERROR_LOG, "ReadFromFile failed: could only read %lu bytes, but %lu bytes were requested", totalBytesRead, numberOfBytesToRead);
		PlatformReadFileResult x;
		x.error = true;
		return x;
		//return {.error = true};
	}
	PlatformReadFileResult x;
	x.string = result;
	return x;
	//return {.string = result};
}

bool PlatformWriteToFile(PlatformFileHandle file, size_t count, const void *buffer) {
	size_t totalBytesWritten = 0;
	ssize_t currentBytesWritten = 0; // Maximum number of bytes that can be returned by a write. (Like size_t, but signed.)
	const char *position = (char *)buffer;
	do {
		currentBytesWritten = write(file, position, (count - totalBytesWritten));
		totalBytesWritten += currentBytesWritten;
		position += currentBytesWritten;
	} while (totalBytesWritten < count && currentBytesWritten != 0);
	if (totalBytesWritten != count) {
		// @TODO: Add file name to file handle.
		LogPrint(ERROR_LOG, "Could not write to file: %s", PlatformGetError());
		return false;
	}
	return true;
}

PlatformFileOffset PlatformGetFileLength(PlatformFileHandle file) {
	struct stat stat; 
	if (fstat(file, &stat) == 0) {
		return (PlatformFileOffset)stat.st_size;
	}
	return FILE_OFFSET_ERROR; 
}

PlatformFileOffset SeekInFile(PlatformFileHandle file, PlatformFileOffset offset, PlatformFileSeekRelative relative) {
	off_t result = lseek(file, offset, relative);
	if (result == (off_t)-1) {
		LogPrint(ERROR_LOG, "File seek failed: %s", PlatformGetError());
	}
	return result;
}

bool PlatformIterateThroughDirectory(const char *path, PlatformDirectoryIteration *context) {
	if (!context->dir) {
		context->dir = opendir(path);
		if (!context->dir) {
			LogPrint(ERROR_LOG, "Failed to open directory %s: %s\n", path, PlatformGetError());
			return false;
		}
	}
	while ((context->dirent = readdir(context->dir))) {
		// @TODO
		if (strcmp(context->dirent->d_name, ".") == 0 || strcmp(context->dirent->d_name, "..") == 0) {
			continue;
		}
		context->filename = context->dirent->d_name;
		context->is_directory = (context->dirent->d_type == DT_DIR);
		return true;
	}
	return false;
}

////////////////////////////////////////
//
// Events.
//

void PlatformHandleWindowEvents(Game_Input *input, GameExecutionStatus *executionStatus) {
	XEvent event;
	XGenericEventCookie *cookie = &event.xcookie;
	XIRawEvent *rawEvent;

	XFlush(linuxContext.display);
	while (XPending(linuxContext.display)) {
		XNextEvent(linuxContext.display, &event);

		if (event.type == ClientMessage && (Atom)event.xclient.data.l[0] == linuxContext.deleteWindowAtom) {
			*executionStatus = GAME_EXITING;
			break;
		}
		if (event.type == ConfigureNotify) {
			XConfigureEvent configureEvent = event.xconfigure;
			// @TODO Window resize.
			break;
		}

		if (!XGetEventData(linuxContext.display, cookie)
		 || cookie->type != GenericEvent
		 || cookie->extension != linuxContext.xinputOpcode) {
			continue;
		}

		rawEvent = (XIRawEvent *)cookie->data;

		switch(rawEvent->evtype) {
		case XI_RawMotion: {
			// @TODO: Check XIMaskIsSet(re->valuators.mask, 0) for x and XIMaskIsSet(re->valuators.mask, 1) for y.
			input->mouse.raw_delta_x += rawEvent->raw_values[0];
			input->mouse.raw_delta_y -= rawEvent->raw_values[1];
		} break;
		case XI_RawKeyPress: {
			press_button(rawEvent->detail, &input->keyboard);
		} break;
		case XI_RawKeyRelease: {
			release_button(rawEvent->detail, &input->keyboard);
		} break;
		case XI_RawButtonPress: {
			u32 buttonIndex = (event.xbutton.button - 1);
			if (buttonIndex > MOUSE_BUTTON_COUNT) {
				break;
			}
			press_button(buttonIndex, &input->mouse.buttons);
		} break;
		case XI_RawButtonRelease: {
			u32 buttonIndex = (event.xbutton.button - 1);
			if (buttonIndex > MOUSE_BUTTON_COUNT) {
				break;
			}
			release_button(buttonIndex, &input->mouse.buttons);
		} break;
		case XI_FocusIn: {
		} break;
		case XI_FocusOut: {
		} break;
		}
	}
}

void PlatformSignalDebugBreakpoint() {
	raise(SIGTRAP);
}

////////////////////////////////////////
//
// Window.
//

void PlatformToggleFullscreen() {
	XEvent event;
	memset(&event, 0, sizeof(event));
	event.xclient.type = ClientMessage;
	event.xclient.window = linuxContext.window;
	event.xclient.message_type = XInternAtom(linuxContext.display, "_NET_WM_STATE", True);
	event.xclient.format = 32;
	event.xclient.data.l[0] = 2;
	event.xclient.data.l[1] = XInternAtom(linuxContext.display, "_NET_WM_STATE_FULLSCREEN", True);
	event.xclient.data.l[2] = 0;
	event.xclient.data.l[3] = 1;
	event.xclient.data.l[4] = 0;
	XSendEvent(linuxContext.display, DefaultRootWindow(linuxContext.display), False, SubstructureRedirectMask | SubstructureNotifyMask, &event);
}

void PlatformCaptureCursor() {
	XDefineCursor(linuxContext.display, linuxContext.window, linuxContext.blankCursor);
	XGrabPointer(linuxContext.display, linuxContext.window, True, 0, GrabModeAsync, GrabModeAsync, None, linuxContext.blankCursor, CurrentTime);
}

void PlatformUncaptureCursor() {
	XUndefineCursor(linuxContext.display, linuxContext.window);
	XUngrabPointer(linuxContext.display, CurrentTime);
}

void PlatformCleanupDisplay() {
	XDestroyWindow(linuxContext.display, linuxContext.window);
	XCloseDisplay(linuxContext.display);
}

////////////////////////////////////////
//
// Memory.
//

#define MAP_ANONYMOUS 0x20

void *PlatformAllocateMemory(size_t size) {
	void *memory = mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (memory == (void *)-1) {
		Assert(0); // @TODO
	}
	return memory;
}

void PlatformFreeMemory(void *memory, size_t size) {
	if (munmap(memory, size) == -1) {
		LogPrint(ERROR_LOG, "Failed to free platform memory: %s\n", PlatformGetError());
	}
}

size_t PlatformGetPageSize() {
	return linuxContext.pageSize;
}

void PlatformPrintStacktrace() {
	LogPrint(INFO_LOG, "Stack trace:\n");
	const u32 addressBufferSize = 100;
	void *addresses[addressBufferSize];
	s32 addressCount = backtrace(addresses, addressBufferSize);
	if (addressCount == addressBufferSize) {
		LogPrint(ERROR_LOG, "Stack trace is probably truncated.\n");
	}
	char **strings = backtrace_symbols(addresses, addressCount);
	if (!strings) {
		LogPrint(ERROR_LOG, "Failed to get function names\n");
		return;
	}
	for (s32 i = 0; i < addressCount; i++) {
		LogPrint(INFO_LOG, "\t%s\n", strings[i]);
	}
	free(strings);
}

////////////////////////////////////////
//
// Vulkan.
//

const char *PlatformGetRequiredVulkanSurfaceInstanceExtension() {
	return "VK_KHR_xlib_surface";
}

void PlatformCreateVulkanSurface(VkInstance instance, VkSurfaceKHR *surface) {
	VkXlibSurfaceCreateInfoKHR surfaceCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
		.dpy = linuxContext.display,
		.window = linuxContext.window,
	};
	VK_CHECK(vkCreateXlibSurfaceKHR(instance, &surfaceCreateInfo, NULL, surface));
}

////////////////////////////////////////
//
// Errors.
//

const char *PlatformGetError() {
	return strerror(errno);
}

s32 PlatformX11ErrorHandler(Display *display, XErrorEvent *event) {
	char buffer[256];
	XGetErrorText(linuxContext.display, event->error_code, buffer, sizeof(buffer));
	Abort("X11 error: %s.", buffer);
	return 0;
}

////////////////////////////////////////
//
// Atomics.
//

s32 PlatformAtomicAddS32(volatile s32 *operand, s32 addend) {
	return __sync_add_and_fetch(operand, addend);
}

s64 PlatformAtomicAddS64(volatile s64 *operand, s64 addend) {
	return __sync_add_and_fetch(operand, addend);
}

s32 PlatformAtomicFetchAndAddS32(volatile s32 *operand, s32 addend) {
	return __sync_fetch_and_add(operand, addend);
}

s32 PlatformAtomicFetchAndAddS64(volatile s64 *operand, s64 addend) {
	return __sync_fetch_and_add(operand, addend);
}

s32 PlatformCompareAndSwapS32(volatile s32 *destination, s32 oldValue, s32 newValue) {
	return __sync_val_compare_and_swap(destination, oldValue, newValue);
}

s64 PlatformCompareAndSwapS64(volatile s64 *destination, s64 oldValue, s64 newValue) {
	return __sync_val_compare_and_swap(destination, oldValue, newValue);
}

void *PlatformCompareAndSwapPointers(void *volatile *destination, void *oldValue, void *newValue) {
	return __sync_val_compare_and_swap(destination, oldValue, newValue);
}

void *PlatformFetchAndSetPointer(void *volatile *target, void *value) {
	return __sync_lock_test_and_set(target, value);
}

////////////////////////////////////////
//
// Threads.
//

s32 PlatformGetProcessorCount() {
	return sysconf(_SC_NPROCESSORS_ONLN);
}

PlatformThreadHandle PlatformCreateThread(PlatformThreadProcedure procedure, void *parameter) {
	pthread_attr_t threadAttributes;
	if (pthread_attr_init(&threadAttributes)) {
		Abort("Failed on pthread_attr_init(): %s", PlatformGetError());
	}
	PlatformThreadHandle thread;
	if (pthread_create(&thread, &threadAttributes, procedure, parameter)) {
		Abort("Failed on pthread_create(): %s", PlatformGetError());
	}
	return thread;
}

PlatformThreadHandle PlatformGetCurrentThread() {
	return pthread_self();
}

void PlatformSetThreadProcessorAffinity(PlatformThreadHandle thread, u32 cpuIndex) {
	cpu_set_t cpuSet;
	CPU_ZERO(&cpuSet);
	CPU_SET(cpuIndex, &cpuSet);
	if (pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuSet)) {
		Abort("Failed on pthread_setaffinity_np(): %s", PlatformGetError());
	}
}

u32 PlatformGetCurrentThreadID() {
	return syscall(__NR_gettid);
}

void PlatformCreateMutex(PlatformMutex *mutex) {
	pthread_mutex_init(mutex, NULL);
}

void PlatformLockMutex(PlatformMutex *mutex) {
	pthread_mutex_lock(mutex);
}

void PlatformUnlockMutex(PlatformMutex *mutex) {
	pthread_mutex_unlock(mutex);
}

////////////////////////////////////////
//
// Fibers.
//
// ucontext_t is the recommended method for implementing fibers on Linux, but it is apparently very
// slow because it preserves each fiber's signal mask.
//
// Our method (from http://www.1024cores.net/home/lock-free-algorithms/tricks/fibers) still creates
// ucontext_t contexts, but it uses _setjmp/_longjmp to switch between them, thus avoiding syscalls
// for saving and setting signal masks.  It works by swapping to the fiber's context during fiber
// creation, setting up a long jump into the new fiber's context using _setjmp, then swapping back
// to the calling context.  Now when we want to switch to the fiber we can _longjmp directly into
// the fiber's context. 

struct FiberCreationInfo {
	PlatformFiberProcedure procedure;
	void *parameter;
	jmp_buf *jumpBuffer;
	ucontext_t *callingContext;
};

void PlatformRunFiber(void *fiberCreationInfoPointer) {
	FiberCreationInfo *fiberCreationInfo = (FiberCreationInfo *)fiberCreationInfoPointer;
	PlatformFiberProcedure procedure = fiberCreationInfo->procedure;
	void *parameter = fiberCreationInfo->parameter;
	if (!_setjmp(*fiberCreationInfo->jumpBuffer)) {
		ucontext_t context = {};
		swapcontext(&context, fiberCreationInfo->callingContext);
	}
	procedure(parameter);
	pthread_exit(NULL);
}

#define FIBER_STACK_SIZE 81920

// @TODO: Rather than mprotecting memory, we should just check to see if the stack pointer is out-of-bound.
void PlatformCreateFiber(PlatformFiber *fiber, PlatformFiberProcedure procedure, void *parameter) {
	getcontext(&fiber->context);
	s32 fiberIndex = PlatformAtomicFetchAndAddS32(&linuxContext.fiberCount, 1);
	fiber->context.uc_stack.ss_sp = linuxContext.fiberStackMemory + ((fiberIndex * FIBER_STACK_SIZE) + ((fiberIndex + 1) * linuxContext.pageSize));
	fiber->context.uc_stack.ss_size = FIBER_STACK_SIZE;
	fiber->context.uc_link = 0;
	ucontext_t temporaryContext;
	FiberCreationInfo fiberCreationInfo = {
		.procedure = procedure,
		.parameter = parameter,
		.jumpBuffer = &fiber->jumpBuffer,
		.callingContext = &temporaryContext,
	};
	makecontext(&fiber->context, (void(*)())PlatformRunFiber, 1, &fiberCreationInfo);
	swapcontext(&temporaryContext, &fiber->context);
}

void PlatformConvertThreadToFiber(PlatformFiber *fiber) {
	threadLocalLinuxContext.activeFiber = fiber;
}

// @TODO: Prevent two fibers from running at the same time.
void PlatformSwitchToFiber(PlatformFiber *fiber) {
	if (!_setjmp(threadLocalLinuxContext.activeFiber->jumpBuffer)) {
		threadLocalLinuxContext.activeFiber = fiber;
		_longjmp(fiber->jumpBuffer, 1);
	}
}

PlatformFiber *PlatformGetCurrentFiber() {
	return threadLocalLinuxContext.activeFiber;
}

////////////////////////////////////////
//
// Semaphores.
//

PlatformSemaphore PlatformCreateSemaphore(u32 initialValue) {
	sem_t semaphore;
	sem_init(&semaphore, 0, initialValue);
	return semaphore;
}

void PlatformSignalSemaphore(PlatformSemaphore *semaphore) {
	sem_post(semaphore);
}

void PlatformWaitOnSemaphore(PlatformSemaphore *semaphore) {
	sem_wait(semaphore);
}

s32 PlatformGetSemaphoreValue(PlatformSemaphore *semaphore) {
	s32 value;
	sem_getvalue(semaphore, &value);
	return value;
}

////////////////////////////////////////
//
// Main.
//

s32 main(s32 argc, char *argv[]) {
	srand(time(0));
	XInitThreads();
	linuxContext.pageSize = sysconf(_SC_PAGESIZE);
	// Install a new error handler.
	// Note this error handler is global.  All display connections in all threads of a process use the same error handler.
	XSetErrorHandler(&PlatformX11ErrorHandler);

	linuxContext.display = XOpenDisplay(NULL);
	if (!linuxContext.display) {
		Abort("Failed to create display");
	}
	s32 screen = XDefaultScreen(linuxContext.display);
	Window rootWindow = XRootWindow(linuxContext.display, screen);

	// Initialize XInput2, which we require for raw input.
	{
		s32 firstEventReturn = 0;
		s32 firstErrorReturn = 0;
		if (!XQueryExtension(linuxContext.display, "XInputExtension", &linuxContext.xinputOpcode, &firstEventReturn, &firstErrorReturn)) {
			Abort("The X server does not support the XInput extension");
		}

		// We are supposed to pass in the minimum version we require to XIQueryVersion, and it passes back what version is available.
		s32 major_version = 2, minor_version = 0;
		XIQueryVersion(linuxContext.display, &major_version, &minor_version);
		if (major_version < 2) {
			Abort("XInput version 2.0 or greater is required: version %d.%d is available", major_version, minor_version);
		}

		u8 mask[] = {0, 0, 0};
		XIEventMask eventMask = {
			.deviceid = XIAllMasterDevices,
			.mask_len = sizeof(mask),
			.mask = mask,
		};
		XISetMask(mask, XI_RawMotion);
		XISetMask(mask, XI_RawButtonPress);
		XISetMask(mask, XI_RawButtonRelease);
		XISetMask(mask, XI_RawKeyPress);
		XISetMask(mask, XI_RawKeyRelease);
		XISetMask(mask, XI_FocusOut);
		XISetMask(mask, XI_FocusIn);
		if (XISelectEvents(linuxContext.display, rootWindow, &eventMask, 1) != Success) {
			Abort("Failed to select XInput events");
		}
	}

	// Create window.
	{
		XVisualInfo visualInfoTemplate = {
			.screen = screen,
		};
		s32 numberOfVisuals;
		XVisualInfo *visualInfo = XGetVisualInfo(linuxContext.display, VisualScreenMask, &visualInfoTemplate, &numberOfVisuals);
		Assert(visualInfo->c_class == TrueColor);

		XSetWindowAttributes windowAttributes = {
			.background_pixel = 0xFFFFFFFF,
			.border_pixmap = None,
			.border_pixel = 0,
			.event_mask = StructureNotifyMask,
			.colormap = XCreateColormap(linuxContext.display, rootWindow, visualInfo->visual, AllocNone),
		};
		s32 windowAttributesMask = CWBackPixel
		                           | CWColormap
		                           | CWBorderPixel
		                           | CWEventMask;
		s32 requested_window_width = 1200;
		s32 requested_window_height = 1000;
		linuxContext.window = XCreateWindow(linuxContext.display,
		                                    rootWindow,
		                                    0,
		                                    0,
		                                    requested_window_width,
		                                    requested_window_height,
		                                    0,
		                                    visualInfo->depth,
		                                    InputOutput,
		                                    visualInfo->visual,
		                                    windowAttributesMask,
		                                    &windowAttributes);
		if (!linuxContext.window) {
			Abort("Failed to create a window");
		}

		XFree(visualInfo);
		XStoreName(linuxContext.display, linuxContext.window, "Jaguar");
		XMapWindow(linuxContext.display, linuxContext.window);
		XFlush(linuxContext.display);

		// Set up the "delete window atom" which signals to the application when the window is closed through the window manager UI.
		if ((linuxContext.deleteWindowAtom = XInternAtom(linuxContext.display, "WM_DELETE_WINDOW", 1))) {
			XSetWMProtocols(linuxContext.display, linuxContext.window, &linuxContext.deleteWindowAtom, 1);
		} else {
			LogPrint(ERROR_LOG, "Unable to register deleteWindowAtom atom.\n");
		}
	}

	// Get actual window dimensions without window borders.
	{
		s32 windowX, windowY;
		u32 borderWidth, depth;
		if (!XGetGeometry(linuxContext.display, linuxContext.window, &rootWindow, &windowX, &windowY, &window_width, &window_height, &borderWidth, &depth)) {
			Abort("failed to get the screen's geometry");
		}
	}

	// Create a blank cursor for when we want to hide the cursor.
	{
		XColor xcolor;
		static char cursor_pixels[] = {0x00};
		Pixmap pixmap = XCreateBitmapFromData(linuxContext.display, linuxContext.window, cursor_pixels, 1, 1);
		linuxContext.blankCursor = XCreatePixmapCursor(linuxContext.display, pixmap, pixmap, &xcolor, &xcolor, 1, 1); 
		XFreePixmap(linuxContext.display, pixmap);
	}

	linuxContext.fiberStackMemory = (char *)PlatformAllocateMemory((JOB_FIBER_COUNT * FIBER_STACK_SIZE) + ((JOB_FIBER_COUNT + 1) * linuxContext.pageSize));
	for (s32 i = 0; i <= JOB_FIBER_COUNT; i++) {
		mprotect(linuxContext.fiberStackMemory + ((i * FIBER_STACK_SIZE) + (i * linuxContext.pageSize)), linuxContext.pageSize, PROT_NONE);
	}

	ApplicationEntry();

	return 0;
}

#if 0
// 
// @TODO: Signal IO errors.
//

char *
get_memory(size_t len)
{
	void *m = mmap(0, len, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (m == (void *)-1)
		Abort("Failed to get memory from platform - %s.", perrno());
	return (char *)m;
}

void
free_memory(void *m, size_t len)
{
	int ret = munmap(m, len);
	if (ret == -1)
		Abort("Failed to free memory from platform - %s.", perrno());
}

size_t
get_page_size()
{
	return sysconf(_SC_PAGESIZE);
}

inline Time_Spec
get_time()
{
	Time_Spec t;
	clock_gettime(CLOCK_MONOTONIC_RAW, &t);
	return t;
}

inline u32
get_time_ms()
{
	Time_Spec t;
	clock_gettime(CLOCK_MONOTONIC_RAW, &t);
	return (t.tv_sec * 1000) + round(t.tv_nsec / 1.0e6);
}

inline u64
get_time_us()
{
	Time_Spec t;
	clock_gettime(CLOCK_MONOTONIC_RAW, &t);
	return (t.tv_sec * 1000000) + round(t.tv_nsec / 1.0e3);
}

// Time in milliseconds.
inline long
time_diff(Time_Spec start, Time_Spec end, unsigned resolution)
{
	ASSERT(0);
	return (end.tv_nsec - start.tv_nsec) / resolution;
}

float
get_seconds_elapsed(Time_Spec start, Time_Spec end)
{
	return (end.tv_sec - start.tv_sec) + ((end.tv_nsec - start.tv_nsec) / 1.0e9);
}

long
keysym_to_codepoint(Key_Symbol keysym)
{
	// First check for Latin-1 characters (1:1 mapping).
	if ((keysym >= 0x0020 && keysym <= 0x007e) || (keysym >= 0x00a0 && keysym <= 0x00ff))
		return keysym;
	// Also check for directly encoded 24-bit unicode characters.
	if ((keysym & 0xff000000) == 0x01000000)
		return keysym & 0x00ffffff;
	// Do a search throught the keysym to unicode mapping table for our keysym.
	// @SPEED: Could make this a binary search or a hash table to improve latency.
	for (size_t i = 0; i < (sizeof(key_symbol_to_unicode) / sizeof(key_symbol_to_unicode[0])); ++i) {
		if (key_symbol_to_unicode[i].key_symbol == keysym)
			return key_symbol_to_unicode[i].unicode;
	}
	// No match.
	return -1;
}

#define READ_AND_ADVANCE_STREAM(type, stream) *((type *)stream); stream += sizeof(type);

struct Pcm_Playback_Info {
	u16 num_channels;
	u32 sample_rate;
	u32 bits_per_sample;
	u32 bytes_per_frame;
	u32 bytes_per_period;
	snd_pcm_uframes_t frames_per_period;
	struct pollfd fd;
} pcm_playback_info;

void
pcm_open_device()
{
	snd_pcm_hw_params_t *pcm_parameters;
	s32                  dir;
	s32                  return_code;

	return_code = snd_pcm_open(&linuxContext.pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
	if (return_code < 0) {
		Abort("Unable to open pcm device: %s.", snd_strerror(return_code));
	}

	pcm_playback_info.num_channels = 2;
	pcm_playback_info.sample_rate = 44100;
	pcm_playback_info.bits_per_sample = 16;
	pcm_playback_info.bytes_per_frame = pcm_playback_info.num_channels * (pcm_playback_info.bits_per_sample / 8);

	snd_pcm_hw_params_alloca(&pcm_parameters);
	snd_pcm_hw_params_any(linuxContext.pcm_handle, pcm_parameters);
	snd_pcm_hw_params_set_access(linuxContext.pcm_handle, pcm_parameters, SND_PCM_ACCESS_RW_INTERLEAVED);
	snd_pcm_hw_params_set_format(linuxContext.pcm_handle, pcm_parameters, SND_PCM_FORMAT_S16_LE);
	snd_pcm_hw_params_set_channels(linuxContext.pcm_handle, pcm_parameters, pcm_playback_info.num_channels);
	snd_pcm_hw_params_set_rate_near(linuxContext.pcm_handle, pcm_parameters, &pcm_playback_info.sample_rate, &dir);

	pcm_playback_info.frames_per_period = 8192;
	snd_pcm_hw_params_set_period_size_near(linuxContext.pcm_handle, pcm_parameters, &pcm_playback_info.frames_per_period, &dir);

	snd_pcm_uframes_t frames_per_buffer = pcm_playback_info.frames_per_period * 2;
	snd_pcm_hw_params_set_buffer_size(linuxContext.pcm_handle, pcm_parameters, frames_per_buffer);

	return_code = snd_pcm_hw_params(linuxContext.pcm_handle, pcm_parameters);
	if (return_code < 0) {
		Abort("Unable to set sound hardware parameters: %s\n", snd_strerror(return_code));
	}

	snd_pcm_hw_params_get_period_size(pcm_parameters, &pcm_playback_info.frames_per_period, &dir);
	snd_pcm_hw_params_get_period_time(pcm_parameters, &pcm_playback_info.sample_rate, &dir);

	pcm_playback_info.bytes_per_period = pcm_playback_info.frames_per_period * pcm_playback_info.bytes_per_frame;

	if(snd_pcm_poll_descriptors(linuxContext.pcm_handle, &pcm_playback_info.fd, 1) < 0) {
		Abort("Error getting file descriptor for PCM.");
	}
}

u8
pcm_less_than_one_period_left_in_buffer()
{
	s32 return_code = poll(&pcm_playback_info.fd, 1, 0);
	if (return_code == -1) {
		log_print(MINOR_ERROR_LOG, "Error polling PCM file descriptor -- %s.", perrno());
	}

	return return_code ? true : false;
}

s32
pcm_write_period(s16 *period_buffer)
{
	s32 frames_written_this_call = snd_pcm_writei(linuxContext.pcm_handle, period_buffer, pcm_playback_info.frames_per_period);
	if (frames_written_this_call == -EPIPE) {
		log_print(MINOR_ERROR_LOG, "PCM underrun occurred.\n");
		snd_pcm_prepare(linuxContext.pcm_handle);
		return -1;
	} else if (frames_written_this_call < 0) {
		log_print(MINOR_ERROR_LOG, "Error from snd_pcm_writei: %s.\n", snd_strerror(frames_written_this_call));
		return -1;
	}

	if ((u32)frames_written_this_call != pcm_playback_info.frames_per_period) {
		log_print(MINOR_ERROR_LOG, "PCM short write, wrote %d frames and expected %d.\n", frames_written_this_call, (s32)pcm_playback_info.frames_per_period);
	}

	return frames_written_this_call;

		//wav_file += frames_written_this_call * bytes_per_frame;
		//frames_written += frames_written_this_call;
#if 0
	auto ma = mem_make_arena();
	String wav_file_string = read_entire_file("../data/sounds/speech.wav", &ma);
	const char *wav_file = wav_file_string.data;

	wav_file += 4;

	u32 wav_file_length = READ_AND_ADVANCE_STREAM(u32, wav_file);

	wav_file += 8;

	u32 format_data_length = READ_AND_ADVANCE_STREAM(u32, wav_file);
	u16 format_type        = READ_AND_ADVANCE_STREAM(u16, wav_file);
	u16 num_channels       = READ_AND_ADVANCE_STREAM(u16, wav_file);
	u32 sample_rate        = READ_AND_ADVANCE_STREAM(u32, wav_file);
	u32 bytes_per_second   = READ_AND_ADVANCE_STREAM(u32, wav_file);
	u16 bytes_per_frame    = READ_AND_ADVANCE_STREAM(u16, wav_file);
	u16 bits_per_sample    = READ_AND_ADVANCE_STREAM(u16, wav_file);

	char chunk_name[5] = {};
	strncpy(chunk_name, wav_file, 4);

	while (strcmp(chunk_name, "data") != 0) {
		wav_file += 4;
		u32 chunk_data_length = READ_AND_ADVANCE_STREAM(u32, wav_file);
		wav_file += chunk_data_length;

		strncpy(chunk_name, wav_file, 4);
	}

	wav_file += 4;

	u32 sample_data_length = READ_AND_ADVANCE_STREAM(u32, wav_file);

	printf("%u\n%u\n%u\n%u\n%u\n%u\n%u\n%u\n", format_data_length, format_type, num_channels, sample_rate, bytes_per_second, bytes_per_frame, bits_per_sample, sample_data_length); 

	s32 total_frames   = sample_data_length / bytes_per_frame;
	s32 frames_written = 0;

	while (frames_written < total_frames) {
		s32 frames_written_this_call = snd_pcm_writei(linuxContext.pcm_handle, wav_file, pcm_playback_info.frames_per_period);
		if (frames_written_this_call == -EPIPE) {
			log_print(MINOR_ERROR_LOG, "PCM underrun occurred.\n");
			snd_pcm_prepare(linuxContext.pcm_handle);
			// @TODO: Exit early?
		} else if (frames_written_this_call < 0) {
			log_print(MINOR_ERROR_LOG, "Error from snd_pcm_writei: %s.\n", snd_strerror(frames_written_this_call));
			// @TODO: Exit early?
		} else {
			if (frames_written_this_call != (s32)pcm_playback_info.frames_per_period) {
				log_print(MINOR_ERROR_LOG, "PCM short write, wrote %d frames and expected %d.\n", frames_written_this_call, (s32)pcm_playback_info.frames_per_period);
			}

			wav_file += frames_written_this_call * bytes_per_frame;
			frames_written += frames_written_this_call;
		}
	}

	snd_pcm_drain(linuxContext.pcm_handle);
	snd_pcm_close(linuxContext.pcm_handle);
#endif
}

void
pcm_close_device()
{
	snd_pcm_drain(linuxContext.pcm_handle);
	snd_pcm_close(linuxContext.pcm_handle);
}

Thread_Handle
create_thread(Thread_Procedure tp, void *thread_argument)
{
	pthread_attr_t attrs;
	if (pthread_attr_init(&attrs)) {
		Abort("Failed on pthread_attr_init(): %s", perrno());
	}

	pthread_t handle;
	if (pthread_create(&handle, &attrs, tp, thread_argument)) {
		Abort("Failed on pthread_create(): %s", perrno());
	}

	return handle;
}

#endif

#if 0
// @TODO: Move job stuff to another file!
#define MAX_JOBS 256
#define NUM_JOB_THREADS 4

typedef void (*Do_Job_Callback)(void *, File);

struct Thread_Job {
	void *job_data;
	Do_Job_Callback do_job_callback;
};

Semaphore_Handle make_semaphore(u32 initial_value);

struct Job_Queue {
	Thread_Job jobs[MAX_JOBS];
	Semaphore_Handle semaphore;
	volatile u32 read_head;
	volatile u32 write_head;
};

Job_Queue job_queue;

void *job_thread_start(void *job_thread_data);

Thread_Handle create_thread(Thread_Procedure tp, void *thread_argument);

void toggle_fullscreen();
#endif
